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

#if defined(DVP_USE_YUV)
#include <yuv/dvp_kl_yuv.h>
#endif

#if defined(DVP_USE_VRUN)
#include <vrun/dvp_kl_vrun.h>
#endif
#include "dvp_unittest_data.h"

#define HERE {printf("=======> UNITTEST - %d <=======\n", __LINE__);fflush(stdout);}

uint32_t iterations;
option_t opts[] = {
    {OPTION_TYPE_INT, &iterations, sizeof(iterations), "-i", "--iter", "Iterations"},
};

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************

static DVP_CoreInfo_t cores[DVP_CORE_MAX];
static DVP_U32 numCoresEnabled;

const DVP_U32 width         = 320;
const DVP_U32 height        = 240;

const char * SERIALIZE    = "SERIALIZE";
const char * UNSERIALIZE  = "UNSERIALIZE";
const char * ALLOC        = "ALLOC";
const char * FREE         = "FREE";
const char * IMPORT       = "IMPORT";
const char * IMPORT_FREE  = "IMPORT_FREE";

//**********************************************************************
// SUPPORT FUNCTIONS
//**********************************************************************

/*! \brief Returns first error in nodes array */
static DVP_Error_e dvp_get_error_from_nodes(DVP_KernelNode_t *nodes,
                                     DVP_U32 numNodes)
{
    DVP_U32 n = 0;
    DVP_Error_e err = DVP_SUCCESS;
    for (n = 0; n < numNodes; n++)
    {
        err = nodes[n].header.error;
        if (err != DVP_SUCCESS)
        {
            return err;
        }
    }
    return err;
}

/*! \brief Prints information when a section completes */
static void dvp_section_complete(void *cookie, DVP_KernelGraph_t *graph, DVP_U32 sectionIndex, DVP_U32 nne)
{
    // just add to the number of nodes executed.
    DVP_U32 *pCount = (DVP_U32 *)cookie;
    DVP_U32 n = 0;

    for (n = 0; n < graph->sections[sectionIndex].numNodes; n++)
    {
        DVP_PRINT(DVP_ZONE_ALWAYS, "Section[%u]:Node[%u] Kernel 0x%x Affinity:%d status=%d\n", sectionIndex, n,
            graph->sections[sectionIndex].pNodes[n].header.kernel,
            graph->sections[sectionIndex].pNodes[n].header.affinity,
            graph->sections[sectionIndex].pNodes[n].header.error);
    }
    *pCount += nne;
}

//**********************************************************************
// UNITTESTS
//**********************************************************************

/*! \brief Tests the creation and destruction of a DVP Handle.
 * \return Returns status_e
 * \retval STATUS_SUCCESS on success.
 * \retval STATUS_FAILURE on failure.
 * \ingroup group_tests
 */
status_e dvp_context_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    if (dvp)
    {
        status = STATUS_SUCCESS;
        DVP_KernelGraph_Deinit(dvp);
    }
    return status;
}

status_e dvp_kernel_node_allocation_test(const char * memOperation)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    DVP_KernelNode_t *node = NULL;
    DVP_U32 numNodes = 16;

    if( dvp )
    {
        node = DVP_KernelNode_Alloc(dvp, numNodes);
        if( node )
        {
            if( !strcmp(memOperation, ALLOC) )
            {
                status = STATUS_SUCCESS;
            }

            DVP_KernelNode_Free(dvp, node, numNodes );

            if( !strcmp(memOperation, FREE) )
            {
                DVP_U32 i;
                void *cmpMem = calloc(1, sizeof(DVP_KernelNode_t));
                status = STATUS_SUCCESS;

                for(i = 0; i < numNodes; i++)
                {
                    if( memcmp(cmpMem, &node[i], sizeof(DVP_KernelNode_t) ) )
                    {
                        status = STATUS_FAILURE;
                    }
                }
            }
        }
        DVP_KernelGraph_Deinit(dvp);
    }
    return status;
}


status_e dvp_kern_node_alloc_test(void)
{
    return dvp_kernel_node_allocation_test(ALLOC);
}

status_e dvp_kern_node_free_test(void)
{
    return dvp_kernel_node_allocation_test(FREE);
}

/*! \brief Tests the \ref DVP_QuerySystem API.
 * \return Returns status_e
 * \retval STATUS_SUCCESS on success.
 * \retval STATUS_FAILURE on failure.
 * \ingroup group_tests
 */
status_e dvp_info_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    if (dvp)
    {
        DVP_U32 c = 0;
        memset(cores, 0, sizeof(cores));
        numCoresEnabled = 0;

        DVP_QuerySystem(dvp, cores);

        for (c = DVP_CORE_MIN+1; c < DVP_CORE_MAX; c++)
        {
            if (cores[c].enabled)
            {
                status = STATUS_SUCCESS;
                numCoresEnabled++;
            }
        }
        DVP_PRINT(DVP_ZONE_ALWAYS, "%u cores enabled!\n",  numCoresEnabled);
        DVP_KernelGraph_Deinit(dvp);
    }
    return status;
}

/*! \brief Tests a single node NO-OP graph on CPU.
 * \return Returns status_e
 * \retval STATUS_SUCCESS on success.
 * \retval STATUS_FAILURE on failure.
 * \ingroup group_tests
 */
status_e dvp_cpu_nop_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    if (dvp)
    {
        DVP_U32 numNodes = 1;
        DVP_U32 numSections = 1;
        DVP_U32 numNodesExecuted = 0;
        DVP_KernelNode_t *nodes = DVP_KernelNode_Alloc(dvp, numNodes);
        if (nodes)
        {
            DVP_KernelGraph_t *graph = DVP_KernelGraph_Alloc(dvp, numSections);
            if (graph)
            {
                DVP_Error_e err = DVP_SUCCESS;
                err = DVP_KernelGraphSection_Init(dvp, graph, 0, nodes, numNodes);
                if (err == DVP_SUCCESS)
                {
                    DVP_U32 numSectionsRun = 0;

                    nodes[0].header.kernel = DVP_KN_NOOP;
                    nodes[0].header.affinity = DVP_CORE_CPU;

                    numSectionsRun = DVP_KernelGraph_Process(dvp, graph, &numNodesExecuted, dvp_section_complete);
                    err = dvp_get_error_from_nodes(nodes, numNodes);
                    if (numSectionsRun == numSections && numNodesExecuted == numNodes && err == DVP_SUCCESS)
                    {
                        status = STATUS_SUCCESS;
                    }
                    DVP_PRINT(DVP_ZONE_ALWAYS, "CPU NOP processed %u sections, %u nodes, first DVP_Error_e=%d\n", numSectionsRun, numNodesExecuted, err);
                }
                DVP_KernelGraph_Free(dvp, graph);
                graph = NULL;
            }
            DVP_KernelNode_Free(dvp, nodes, numNodes);
            nodes = NULL;
        }
        DVP_KernelGraph_Deinit(dvp);
    }
    return status;
}


/*! \brief Tests a serial array of nodes graph on all available cores.
 * \return Returns status_e
 * \retval STATUS_SUCCESS on success.
 * \retval STATUS_FAILURE on failure.
 * \ingroup group_tests
 */
status_e dvp_serial_nop_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    if (dvp)
    {
        DVP_U32 numSections = 1;
        DVP_U32 numNodes = numCoresEnabled;
        DVP_U32 numNodesExecuted = 0;
        DVP_KernelNode_t *nodes = DVP_KernelNode_Alloc(dvp, numNodes);
        if (nodes)
        {
            DVP_KernelGraph_t *graph = DVP_KernelGraph_Alloc(dvp, numSections);
            if (graph)
            {
                DVP_Error_e err = DVP_SUCCESS;
                err = DVP_KernelGraphSection_Init(dvp, graph, 0, nodes, numNodes);
                if (err == DVP_SUCCESS)
                {
                    DVP_U32 n, numSectionsRun = 0;
                    DVP_Core_e c;

                    for (n = 0, c = DVP_CORE_MIN+1; c < DVP_CORE_MAX; c++)
                    {
                        if (cores[c].enabled == DVP_TRUE)
                        {
                            nodes[n].header.kernel = DVP_KN_NOOP;
                            nodes[n].header.affinity = (DVP_Core_e)c;
                            n++;
                        }
                    }

                    numSectionsRun = DVP_KernelGraph_Process(dvp, graph, &numNodesExecuted, dvp_section_complete);
                    err = dvp_get_error_from_nodes(nodes, numNodes);
                    if (numSectionsRun == numSections && numNodesExecuted == numNodes && err == DVP_SUCCESS)
                    {
                        status = STATUS_SUCCESS;
                    }
                    DVP_PRINT(DVP_ZONE_ALWAYS, "SERIAL NOP processed %u sections, %u nodes, first DVP_Error_e=%d\n", numSectionsRun, numNodesExecuted, err);
                }
                DVP_KernelGraph_Free(dvp, graph);
                graph = NULL;
            }
            DVP_KernelNode_Free(dvp, nodes, numNodes);
            nodes = NULL;
        }
        DVP_KernelGraph_Deinit(dvp);
        dvp = 0;
    }
    return status;
}

/*! \brief Tests a "parallel set of nodes" graph on all available cores.
 * \return Returns status_e
 * \retval STATUS_SUCCESS on success.
 * \retval STATUS_FAILURE on failure.
 * \ingroup group_tests
 */
status_e dvp_parallel_nop_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    if (dvp)
    {
        DVP_U32 numNodes = numCoresEnabled;
        DVP_U32 numSections = numCoresEnabled;
        DVP_U32 numNodesExecuted = 0;
        DVP_KernelNode_t *nodes = DVP_KernelNode_Alloc(dvp, numNodes);
        if (nodes)
        {
            DVP_KernelGraph_t *graph = DVP_KernelGraph_Alloc(dvp, numSections);
            if (graph)
            {
                DVP_Error_e err = DVP_SUCCESS;
                DVP_U32 n, s = 0;
                DVP_Core_e c;

                // one node per section...
                for (s = 0; s < numSections; s++)
                {
                    err = DVP_KernelGraphSection_Init(dvp, graph, s, &nodes[s], 1);
                    if (err != DVP_SUCCESS)
                    {
                        DVP_KernelGraph_Free(dvp, graph);
                        DVP_KernelNode_Free(dvp, nodes, numNodes);
                        return STATUS_NOT_ENOUGH_MEMORY;
                    }
                }

                if (err == DVP_SUCCESS)
                {
                    DVP_U32 numSectionsRun = 0;

                    // set the core affinity to all the available cores.
                    for (n = 0, c = DVP_CORE_MIN+1; c < DVP_CORE_MAX; c++)
                    {
                        if (cores[c].enabled == DVP_TRUE)
                        {
                            nodes[n].header.kernel = DVP_KN_NOOP;
                            nodes[n].header.affinity = (DVP_Core_e)c;
                            n++;
                        }
                    }

                    numSectionsRun = DVP_KernelGraph_Process(dvp, graph, &numNodesExecuted, dvp_section_complete);
                    err = dvp_get_error_from_nodes(nodes, numNodes);
                    if (numSectionsRun == numSections && numNodesExecuted == numNodes && err == DVP_SUCCESS)
                    {
                        status = STATUS_SUCCESS;
                    }
                    DVP_PRINT(DVP_ZONE_ALWAYS, "PARALLEL NOP processed %u sections, %u nodes, first DVP_Error_e=%d\n", numSectionsRun, numNodesExecuted, err);
                }
                DVP_KernelGraph_Free(dvp, graph);
                graph = NULL;
            }
            DVP_KernelNode_Free(dvp, nodes, numNodes);
            nodes = NULL;
        }
        DVP_KernelGraph_Deinit(dvp);
    }
    return status;
}

/*! \brief Tests a set of copy nodes in series on all available cores.
 * \return Returns status_e
 * \retval STATUS_SUCCESS on success.
 * \retval STATUS_FAILURE on failure.
 * \ingroup group_tests
 */
status_e dvp_copy_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_MemType_e mtype = DVP_MTYPE_DEFAULT;
    DVP_Handle dvp = DVP_KernelGraph_Init();
#if defined(DVP_USE_BO) || defined(DVP_USE_ION)
    mtype = DVP_MTYPE_MPUCACHED_1DTILED;
#endif
    if (dvp)
    {
        DVP_U32 numSections = 1;
        DVP_U32 numNodes = numCoresEnabled;
        DVP_U32 numNodesExecuted = 0;
        DVP_KernelNode_t *nodes = DVP_KernelNode_Alloc(dvp, numNodes);
        if (nodes)
        {
            DVP_KernelGraph_t *graph = DVP_KernelGraph_Alloc(dvp, numSections);
            if (graph)
            {
                DVP_Error_e err = DVP_SUCCESS;
                err = DVP_KernelGraphSection_Init(dvp, graph, 0, nodes, numNodes);
                if (err == DVP_SUCCESS)
                {
#define width 320
#define height 240
                    DVP_U32 n, numSectionsRun = 0;
                    DVP_Core_e c;
                    DVP_U08 ptr[height][width];
                    memset(ptr, 0x57, sizeof(ptr));

                    for (n = 0, c = 0; c < DVP_CORE_MAX; c++)
                    {
                        if (cores[c].enabled == DVP_TRUE)
                        {
                            DVP_Transform_t *pT = dvp_knode_to(&nodes[n], DVP_Transform_t);
                            nodes[n].header.kernel = DVP_KN_COPY;
                            nodes[n].header.affinity = (DVP_Core_e)c;

                            DVP_Image_Init(&pT->input, width, height, FOURCC_Y800);
                            DVP_Image_Alloc(dvp, &pT->input, mtype);

                            DVP_Image_Fill(&pT->input, (DVP_S08 *)ptr, width*height*sizeof(DVP_U08));

                            DVP_Image_Init(&pT->output, width, height, FOURCC_Y800);
                            DVP_Image_Alloc(dvp, &pT->output, mtype);

                            n++;
                        }
                    }

                    numSectionsRun = DVP_KernelGraph_Process(dvp, graph, &numNodesExecuted, dvp_section_complete);
                    err = dvp_get_error_from_nodes(nodes, numNodes);

                    for (n = 0; n < numNodes; n++)
                    {
                        DVP_Transform_t *pT = dvp_knode_to(&nodes[n], DVP_Transform_t);
                        if (DVP_Image_Equal(&pT->output, &pT->input) == DVP_FALSE)
                        {
                            DVP_PRINT(DVP_ZONE_ERROR, "Output != Input on Node %d, Affinity %d\n", n, nodes[n].header.affinity);
                            err = DVP_ERROR_FAILURE;
                        }
                        DVP_Image_Free(dvp, &pT->input);
                        DVP_Image_Free(dvp, &pT->output);
                    }

                    if (numSectionsRun == numSections && numNodesExecuted == numNodes && err == DVP_SUCCESS)
                    {
                        status = STATUS_SUCCESS;
                    }
                    DVP_PRINT(DVP_ZONE_ALWAYS, "Serial Copy processed %u sections, %u nodes, first DVP_Error_e=%d\n", numSectionsRun, numNodesExecuted, err);
                }
                DVP_KernelGraph_Free(dvp, graph);
                graph = NULL;
            }
            DVP_KernelNode_Free(dvp, nodes, numNodes);
            nodes = NULL;
        }
        DVP_KernelGraph_Deinit(dvp);
        dvp = 0;
    }
    return status;
#undef width
#undef height
}

/*! \brief Tests a serial/parallel/serial copy graph on all available cores.
 * \return Returns status_e
 * \retval STATUS_SUCCESS on success.
 * \retval STATUS_FAILURE on failure.
 * \ingroup group_tests
 */
status_e dvp_custom_copy_test(void)
{
    status_e status = STATUS_FAILURE;

    DVP_U32 numSections = 2 + numCoresEnabled;
    DVP_U32 numNodes = 3 * numCoresEnabled;

    DVP_U32 section_index = 0, image_index = 0, in_index;
    DVP_KernelNode_t *section_node;

    DVP_U32 numNodesExecuted = 0;
    DVP_U32 numSectionsRun = 0;

    DVP_Core_e c;
    DVP_Transform_t *pT;

    DVP_U32 width = 640, height = 480;
    DVP_U32 numImages = numNodes + 1;
    DVP_Image_t images[numImages];

    DVP_U32 n;
    DVP_Error_e err = DVP_SUCCESS;

    DVP_U08 ptr[height][width];
    memset(ptr, 0x57, sizeof(ptr));


    DVP_Handle dvp = DVP_KernelGraph_Init();
    if (dvp == 0)
        return status;

    DVP_KernelNode_t *nodes = DVP_KernelNode_Alloc(dvp, numNodes);
    if (nodes == NULL)
    {
        DVP_KernelGraph_Deinit(dvp);
        return STATUS_NOT_ENOUGH_MEMORY;
    }

    DVP_KernelGraph_t *graph = DVP_KernelGraph_Alloc(dvp, numSections);
    if (graph == NULL)
    {
        DVP_KernelNode_Free(dvp, nodes, numNodes);
        DVP_KernelGraph_Deinit(dvp);
        return STATUS_NOT_ENOUGH_MEMORY;
    }

    for (n = 0; n < numImages; n++)
    {
        DVP_Image_Init(&images[n], width, height, FOURCC_Y800);
        DVP_Image_Alloc(dvp, &images[n], DVP_MTYPE_DEFAULT);
    }

    section_node = nodes;

    err = DVP_KernelGraphSection_Init(dvp, graph, section_index, section_node, numCoresEnabled);
    if (err != DVP_SUCCESS)
    {
        goto exit;
    }
    graph->order[section_index] = 0;

    for (n = 0; n < numCoresEnabled; n++)
    {
        pT = dvp_knode_to(&section_node[n], DVP_Transform_t);
        pT->input = images[image_index];
        pT->output = images[image_index + 1];

        image_index ++;
    }

    section_index ++;
    section_node += numCoresEnabled;

    in_index = image_index;

    // one node per section...
    for (n = 0; n < numCoresEnabled; n++)
    {
        image_index ++;

        err = DVP_KernelGraphSection_Init(dvp, graph, section_index, section_node, 1);
        if (err != DVP_SUCCESS)
        {
            status = STATUS_NOT_ENOUGH_MEMORY;
            goto exit;
        }
        graph->order[section_index] = 1;

        pT = dvp_knode_to(section_node, DVP_Transform_t);
        pT->input = images[in_index];
        pT->output = images[image_index];

        section_node ++;
        section_index ++;
    }

    err = DVP_KernelGraphSection_Init(dvp, graph, section_index, section_node, numCoresEnabled);
    if (err != DVP_SUCCESS)
    {
        goto exit;
    }
    graph->order[section_index] = 2;

    for (n = 0; n < numCoresEnabled; n++)
    {
        pT = dvp_knode_to(&section_node[n], DVP_Transform_t);
        pT->input = images[image_index];
        pT->output = images[image_index + 1];

        image_index ++;
    }

    // set to last image
    section_node += numCoresEnabled - 1;

    for (n = 0, c = DVP_CORE_MIN+1; c < DVP_CORE_MAX; c++)
    {
        if (cores[c].enabled == DVP_TRUE)
        {
            nodes[n].header.kernel = DVP_KN_COPY;
            nodes[n].header.affinity = (DVP_Core_e)c;

            nodes[n + numCoresEnabled].header.kernel = DVP_KN_COPY;
            nodes[n + numCoresEnabled].header.affinity = (DVP_Core_e)c;

            nodes[n + 2 * numCoresEnabled].header.kernel = DVP_KN_COPY;
            nodes[n + 2 * numCoresEnabled].header.affinity = (DVP_Core_e)c;

            n++;
        }
    }

    pT = dvp_knode_to(&nodes[0], DVP_Transform_t);
    DVP_Image_Fill(&pT->input, (DVP_S08 *)ptr, width*height*sizeof(DVP_U08));

    numSectionsRun = DVP_KernelGraph_Process(dvp, graph, &numNodesExecuted, dvp_section_complete);

    err = dvp_get_error_from_nodes(nodes, numNodes);
    if (numSectionsRun == numSections && numNodesExecuted == numNodes && err == DVP_SUCCESS)
    {
        DVP_Transform_t *pT1 = dvp_knode_to(section_node, DVP_Transform_t);
        if (DVP_Image_Equal(&pT->input, &pT1->output) == DVP_FALSE)
        {
            DVP_PRINT(DVP_ZONE_ERROR, "dvp_custom_copy_test: Output != Input\n");
        }
        else
            status = STATUS_SUCCESS;
    }

    DVP_PRINT(DVP_ZONE_ALWAYS, "CUSTOM COPY processed %u sections, %u nodes, first DVP_Error_e=%d\n", numSectionsRun, numNodesExecuted, err);

exit:
    for (n = 0; n < numImages; n++)
    {
        DVP_Image_Free(dvp, &images[n]);
        DVP_Image_Deinit(&images[n]);
    }

    DVP_KernelGraph_Free(dvp, graph);
    DVP_KernelNode_Free(dvp, nodes, numNodes);
    DVP_KernelGraph_Deinit(dvp);

    return status;
}


#if defined(DVP_USE_YUV)
/*! \brief Tests a split image color conversion.
 * \return Returns status_e
 * \retval STATUS_SUCCESS on success.
 * \retval STATUS_FAILURE on failure.
 * \ingroup group_tests
 */
status_e dvp_split_cc_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    if (dvp)
    {
        DVP_U32 numNodes = 2;
        DVP_U32 numSections = 2;
        DVP_U32 numNodesExecuted = 0;
        DVP_KernelNode_t *nodes = DVP_KernelNode_Alloc(dvp, numNodes);
        if (nodes)
        {
            DVP_KernelGraph_t *graph = DVP_KernelGraph_Alloc(dvp, numSections);
            if (graph)
            {
                DVP_Error_e err = DVP_SUCCESS;
                err = DVP_KernelGraphSection_Init(dvp, graph, 0, nodes, numNodes);
                if (err == DVP_SUCCESS)
                {
                    DVP_Image_t images[2];
                    DVP_U32 numSectionsRun = 0;
                    DVP_Transform_t *io[2];
                    DVP_Image_t *img[4];

                    DVP_Image_Init(&images[0], 1280, 720, FOURCC_ARGB);
                    DVP_Image_Init(&images[1], 1280, 720, FOURCC_UYVY);

                    DVP_Image_Alloc(dvp, &images[0], DVP_MTYPE_MPUCACHED_VIRTUAL);
                    DVP_Image_Alloc(dvp, &images[1], DVP_MTYPE_MPUCACHED_VIRTUAL);

                    nodes[0].header.kernel = DVP_KN_YUV_ARGB_TO_UYVY;
                    nodes[0].header.affinity = DVP_CORE_CPU;
                    io[0] = dvp_knode_to(&nodes[0],DVP_Transform_t);
                    DVP_Image_Dup(&io[0]->input, &images[0]);
                    DVP_Image_Dup(&io[0]->output, &images[1]);

                    nodes[1].header.kernel = DVP_KN_YUV_ARGB_TO_UYVY;
                    nodes[1].header.affinity = DVP_CORE_CPU;
                    io[1] = dvp_knode_to(&nodes[1],DVP_Transform_t);
                    DVP_Image_Dup(&io[1]->input, &images[0]);
                    DVP_Image_Dup(&io[1]->output, &images[1]);

                    // now perform some arimethic on the pointers to make each
                    // node cover half the image.
                    img[0] = &io[0]->input;
                    img[1] = &io[0]->output;
                    img[2] = &io[1]->input;
                    img[3] = &io[1]->output;

                    img[0]->height /= 2;
                    img[1]->height /= 2;
                    img[2]->height /= 2;
                    img[3]->height /= 2;

                    // move the pointer 1/2 down the image
                    img[2]->pData[0] = &img[2]->pData[0][img[2]->height * img[2]->y_stride];
                    img[3]->pData[0] = &img[3]->pData[0][img[3]->height * img[3]->y_stride];

                    // assign each node to a separate section
                    DVP_KernelGraphSection_Init(dvp, graph, 0, &nodes[0], 1);
                    DVP_KernelGraphSection_Init(dvp, graph, 1, &nodes[1], 1);

                    numSectionsRun = DVP_KernelGraph_Process(dvp, graph, &numNodesExecuted, dvp_section_complete);
                    err = dvp_get_error_from_nodes(nodes, numNodes);
                    if (numSectionsRun == numSections && numNodesExecuted == numNodes && err == DVP_SUCCESS)
                    {
                        status = STATUS_SUCCESS;
                    }

                    DVP_Image_Free(dvp, &images[0]);
                    DVP_Image_Free(dvp, &images[1]);

                    DVP_PRINT(DVP_ZONE_ALWAYS, "CPU CC processed %u sections, %u nodes, first DVP_Error_e=%d\n", numSectionsRun, numNodesExecuted, err);
                    DVP_PrintPerformanceGraph(dvp, graph);
                }
                DVP_KernelGraph_Free(dvp, graph);
                graph = NULL;
            }
            DVP_KernelNode_Free(dvp, nodes, numNodes);
            nodes = NULL;
        }
        DVP_KernelGraph_Deinit(dvp);
    }
    return status;
}
#endif
/*!
 * \brief Tests core capacity APIs
 * \return Returns status_e
 * \retval STATUS_SUCCESS on success.
 * \retval STATUS_FAILURE on failure.
 * \ingroup group_ tests
 */
status_e dvp_capacity_test(void)
{
    /// @todo check if it's actually needed returning failure in case any the the cores culdn't be set, while some could
    /// @todo depends on the pourposes of the test
    status_e status = STATUS_FAILURE;
    DVP_S32 core;

    DVP_Handle dvp = DVP_KernelGraph_Init();
    if( dvp )
    {
        DVP_U32 cap  = DVP_CAPACITY_DATA_RANGE * 3 / 4;
        for( core= DVP_CORE_MIN  + 1; core < DVP_CORE_MAX ; core++  )
        {
            DVP_SetCoreCapacity(dvp, core, cap);
            DVP_PRINT(DVP_ZONE_ALWAYS, "Core Capacity set to %d on core ID %d \n", cap, core);

            if ( cap == DVP_GetCoreCapacity(dvp, core) )
            {
                DVP_PRINT(DVP_ZONE_ALWAYS, "Set/Get core capacity successful for core id %d\n", core);
                status = STATUS_SUCCESS;
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ALWAYS, "Set/Get core capacity failed for core id %d\n", core);
            }
        }
        DVP_KernelGraph_Deinit(dvp);
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ALWAYS, "Failed to initialize DVP Kernel Graph!!!\n");
        status = STATUS_FAILURE;
    }

    return status;
}

status_e dvp_query_kernel_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_KernelNode_e kernel = DVP_KF_DEBUG_BASE + 1;
    DVP_BOOL present[DVP_CORE_MAX], check[DVP_CORE_MAX];

    DVP_Handle dvp = DVP_KernelGraph_Init();
    if( dvp )
    {
        DVP_U32 feature_ends_at[] = {
            DVP_KN_BGR3_TO_IYUV,
            DVP_KN_PREWITT_8,
            DVP_KN_ERODE_SQUARE,
            DVP_KN_CANNY_HYST_THRESHHOLD,
            DVP_KN_XSTRIDE_SHIFT,
            DVP_KN_IIR_VERT,
            DVP_KN_NONMAXSUPPRESS_7x7_S16,
            DVP_KN_CONV_7x7,
            DVP_KN_THR_LE2THR_16,
            DVP_KN_SOBEL_7x7_16,
            DVP_KN_GAMMA
        };
        DVP_KernelFeature_e feature;
        DVP_KernelFeature_e feature_start   = DVP_KF_COLOR_CONVERT;
        DVP_KernelFeature_e feature_end     = DVP_KF_INTEGRAL + 1;
        for( feature = feature_start; feature < feature_end; feature++ )
        {
            for(kernel = (DVP_KN_FEATURE_BASE(feature) + 1); kernel <= (feature_ends_at[feature - feature_start]); kernel++)
            {
                memset(present, (DVP_FALSE - 1), sizeof(present)   );
                memset(check,   (DVP_FALSE - 1), sizeof(check)     );
                DVP_QueryKernel(dvp, kernel, present);
                DVP_U32 i;
                for( i = 0; i < dimof(present) ; i++ )
                {
                    if( DVP_TRUE == present[i] ) {
                        status = STATUS_SUCCESS;
                        break;
                    }
                    else if( DVP_FALSE == present[i] ) {
                        continue;
                    }
                    else {
                        // possible momory corruption !!!
                        DVP_PRINT(DVP_ZONE_ALWAYS, "A boolean API returned non boolean value!\n");
                        DVP_PRINT(DVP_ZONE_ALWAYS, "Possible momory corruption !!!");
                        status = STATUS_FAILURE;
                        break;
                    }
                }
            }
        }
        DVP_KernelGraph_Deinit(dvp);
    }
    else
    {
        DVP_PRINT( DVP_ZONE_ALWAYS, "Failed to initialize DVP Kernel Graph!!!\n");
        status = STATUS_FAILURE;
    }

    return status;
}

status_e dvp_image_init_test(void)
{
    status_e res = STATUS_FAILURE;
    DVP_Image_t *pImage = malloc(sizeof(DVP_Image_t));
    if(pImage)
    {
        DVP_Image_Init(pImage, width, height, FOURCC_NV12);
        DVP_U32 expectedSize = (height * width * 3u)/2u;
        if( pImage->color == FOURCC_NV12
                && width == pImage->width && height == pImage->height
                && pImage->bufWidth == width && pImage->bufHeight == height
                && expectedSize == pImage->numBytes && 2 == pImage->planes )
        {
            res = STATUS_SUCCESS;
        }
        free(pImage);
    }
    return res;
}

status_e dvp_image_allocation_test(const char *memOperation)
{
    status_e res = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    if(dvp )
    {
        pImage = malloc(sizeof(DVP_Image_t));
        if(pImage)
        {
            DVP_Image_Init(pImage, width, height, FOURCC_NV12);
            if ( DVP_TRUE == DVP_Image_Alloc(dvp, pImage, DVP_MTYPE_DEFAULT) )
            {
                if( !strcmp(memOperation, ALLOC) )
                {
                    res = STATUS_SUCCESS;
                }

                if( DVP_TRUE == DVP_Image_Free(dvp, pImage) && !strcmp(memOperation, FREE) )
                {
                    res = STATUS_SUCCESS;
                }
            }
            free(pImage);
        }
        DVP_KernelGraph_Deinit(dvp);
    }

    return res;
}

status_e dvp_image_alloc_test(void)
{
    return dvp_image_allocation_test(ALLOC);
}

status_e dvp_image_free_test(void)
{
    return dvp_image_allocation_test(FREE);
}

status_e dvp_image_share_test(void)
{
    status_e stat = STATUS_FAILURE;
    DVP_S08 mem[width*height];
    DVP_Image_t *pImage = malloc(sizeof(DVP_Image_t));
    DVP_Handle dvp = DVP_KernelGraph_Init();

    if( dvp )
    {
        if( pImage )
        {
            memset(mem, 0x57, sizeof(mem));

            DVP_Image_Init(pImage, width, height, FOURCC_Y800);
            pImage->memType = DVP_MTYPE_DEFAULT;

            if( DVP_TRUE == DVP_Image_Alloc(dvp, pImage, (DVP_MemType_e)(pImage->memType)) )
            {
                DVP_Image_Fill(pImage, mem, sizeof(mem));

                DVP_S32 fds[1] = { 0 };
                if( DVP_TRUE == DVP_Image_Share( dvp, pImage, fds ) )
                {
                    stat = STATUS_SUCCESS;
                }
                DVP_Image_Free(dvp, pImage);
            }
            DVP_Image_Deinit(pImage);
            free(pImage);
        }
        DVP_KernelGraph_Deinit(dvp);
    }

    return stat;
}

status_e dvp_image_importer_test(const char* importOperation)
{
    status_e status = STATUS_FAILURE;
    DVP_S08 mem[width*height];

    DVP_Image_t *pImage = malloc(sizeof(DVP_Image_t));

    DVP_Handle dvp_importer = DVP_MemImporter_Create();
    DVP_Handle dvp_allocator = DVP_KernelGraph_Init();

    if( dvp_importer && dvp_allocator )
    {
        if( pImage )
        {
            memset(mem, 0x57, sizeof(mem));

            DVP_Image_Init(pImage, width, height, FOURCC_Y800);
            pImage->memType = DVP_MTYPE_DEFAULT;

            if( DVP_TRUE == DVP_Image_Alloc(dvp_allocator, pImage, (DVP_MemType_e)(pImage->memType)) )
            {
                DVP_Image_Fill(pImage, mem, sizeof(mem));

                DVP_S32 fds[1] = { 0 };
                if( DVP_TRUE == DVP_Image_Share( dvp_allocator, pImage, fds ) )
                {
                    DVP_VALUE hdls[1] = { 0 };
                    if( DVP_TRUE == DVP_Image_Import(dvp_importer, pImage, fds, hdls) )
                    {
                        DVP_PrintImage(DVP_ZONE_ALWAYS, pImage);
                        if( !strcmp(importOperation, IMPORT) )
                        {
                            status = STATUS_SUCCESS;
                        }

                        if( DVP_TRUE == DVP_Image_Free_Import(dvp_importer, pImage, hdls) && !strcmp(importOperation, IMPORT_FREE))
                        {
                            status = STATUS_SUCCESS;
                        }
                    }
                    else
                    {
                        DVP_PRINT(DVP_ZONE_ALWAYS, "DVP_Image_Import() failed !!!\n");
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ALWAYS, "DVP_Image_Share() failed !!!\n");
                }

                DVP_Image_Free(dvp_allocator, pImage);
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ALWAYS, "Failed to allocate DVP_Image!!!\n");
            }

            DVP_Image_Deinit(pImage);

            free(pImage);
        }
        DVP_MemImporter_Free(dvp_importer);
        DVP_KernelGraph_Deinit(dvp_allocator);
    }

    return status;
}

status_e dvp_image_import_test(void)
{
    return dvp_image_importer_test(IMPORT);
}

status_e dvp_image_import_free_test(void)
{
    return dvp_image_importer_test(IMPORT_FREE);
}

status_e dvp_calloc_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    DVP_Image_t *imgPtr = NULL;

    if( dvp )
    {
        imgPtr = (DVP_Image_t*)dvp_calloc(dvp, 1, sizeof(DVP_Image_t));
        if( imgPtr )
        {
            dvp_free( dvp, imgPtr, 1, sizeof(DVP_Image_t));
        }
        DVP_KernelGraph_Deinit(dvp);
    }

    return status;
}

status_e dvp_img_copy_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_S32 success  = 0, fail = 0;

    DVP_PRINT(DVP_ZONE_ALWAYS, "Starting DVP_Image_Copy() test.\n");
    DVP_Handle dvp = DVP_KernelGraph_Init();

    DVP_Image_t *srcImage = malloc(sizeof(DVP_Image_t));
    DVP_Image_t *dstImage = malloc(sizeof(DVP_Image_t));

    if(!dvp || !srcImage || !dstImage)
    {
        if( dvp ) {
            DVP_KernelGraph_Deinit(dvp);
            dvp = 0;
        }
        if( srcImage ) {
            free(srcImage);
            srcImage = NULL;
        }
        if( dstImage ) {
            free(dstImage);
            dstImage = NULL;
        }

        return STATUS_NOT_ENOUGH_MEMORY;
    }

    if( dvp && srcImage && dstImage )
    {
        DVP_U32 copyCnt;
        for( copyCnt = 0; copyCnt < dimof(img_copy_conversions); copyCnt++ )
        {
            DVP_Image_Init(srcImage, width, height, img_copy_conversions[copyCnt].from);
            if( DVP_FALSE == DVP_Image_Alloc(dvp, srcImage, DVP_MTYPE_DEFAULT) )
            {
                DVP_Image_Deinit(srcImage);
                continue;
            }

            DVP_S08 *ptr = malloc(srcImage->numBytes);
            memset(ptr, 0x57, srcImage->numBytes);

            if( NULL == ptr ) {
                DVP_Image_Free(dvp, srcImage);
                DVP_Image_Deinit(srcImage);
                continue;
            }

            DVP_Image_Fill(srcImage, ptr, srcImage->numBytes);

            DVP_Image_Init(dstImage, width, height, img_copy_conversions[copyCnt].to);

            if ( DVP_Image_Alloc(dvp, dstImage, DVP_MTYPE_DEFAULT) )
            {
                DVP_U32 i;
                DVP_S32 fromInd = -1, toInd = -1;
                for( i = 0u; i < dimof(color_name_table); i++ )
                {
                    if( color_name_table[i].colorID == img_copy_conversions[copyCnt].from )
                        fromInd = i;

                    if( color_name_table[i].colorID == img_copy_conversions[copyCnt].to )
                        toInd = i;

                    if( fromInd >= 0  && toInd >= 0 )
                    {
                        DVP_PRINT(DVP_ZONE_ALWAYS,  "Formats for image copy: %s to %s.\n",
                                      color_name_table[fromInd].colorName, color_name_table[toInd].colorName );
                        break;
                    }
                }

                if ( DVP_TRUE == DVP_Image_Copy(dstImage, srcImage) )
                {
                    DVP_PRINT(DVP_ZONE_ALWAYS,  "(image copied...)\n");
                    if ( DVP_Image_Equal(dstImage, srcImage))
                    {
                        status = STATUS_SUCCESS;
                        DVP_PRINT(DVP_ZONE_ALWAYS,  "DVP_Image_Copy() : Success\n");
                        success++;
                    }
                    else
                    {
                        status = STATUS_FAILURE;
                        fail++;
                        DVP_PRINT(DVP_ZONE_ALWAYS,  "DVP_Image_Copy() : Failure\n");
                    }
                }
                else
                {
                    status = STATUS_FAILURE;
                    fail++;
                    DVP_PRINT(DVP_ZONE_ALWAYS,  "DVP_Image_Copy() : Failure\n");
                }

                DVP_Image_Free(dvp, dstImage);
            }
            DVP_Image_Deinit(dstImage);

            DVP_Image_Free(dvp, srcImage);
            DVP_Image_Deinit(srcImage);

            free(ptr);
        }
    }

#ifdef DVP_DEBUG
    DVP_S32 avg = (success * 100) / (success + fail);
    DVP_PRINT(DVP_ZONE_ALWAYS,  "DVP_Image_Print() has ended with average success of %d %% \n", avg);
#endif // DVP_DEGBUG

    if( dvp ) {
        DVP_KernelGraph_Deinit(dvp);
        dvp = 0;
    }

    if( srcImage ) {
        free(srcImage);
        srcImage = NULL;
    }

    if( dstImage ) {
        free(dstImage);
        dstImage = NULL;
    }

    return status;
}

status_e dvp_image_serialization(const char *testVariant)
{
#define num_imgs_max (21)
#define max_mem_per_img (1024*512)
#define CMP_VAL 0xEB

    status_e    ret                 = STATUS_SUCCESS;
    DVP_BOOL    use_single_image    = DVP_TRUE;
    DVP_U08     *buffer             = NULL;
    DVP_Image_t *pImg_Ser[num_imgs_max];
    DVP_Handle  dvp = DVP_KernelGraph_Init();

    if( dvp )
    {
        DVP_S32 count, alloc_size = 0;

        memset(pImg_Ser, 0, sizeof(pImg_Ser));
        for( count = 0; count < num_imgs_max; count++ )
        {
//            pImg_Ser[count] = NULL;
            pImg_Ser[count] = malloc(sizeof(DVP_Image_t));

            if( NULL == pImg_Ser[count] )
            {
                ret = STATUS_NOT_ENOUGH_MEMORY;
                break;
            }

            DVP_Image_Init(pImg_Ser[count], width, height, FOURCC_NV12);

            if( DVP_TRUE == DVP_Image_Alloc(dvp, pImg_Ser[count], DVP_MTYPE_DEFAULT) )
            {
                if( DVP_TRUE == use_single_image ) {
                    alloc_size = max_mem_per_img * num_imgs_max;
                    break; // quit this loop once we get here...
                }
                else {
                    alloc_size += max_mem_per_img;
                }
            }
            else
            {
                ret = STATUS_NOT_ENOUGH_MEMORY;
                break;
            }
        }

        if( alloc_size && STATUS_SUCCESS == ret )
        {
            buffer = malloc(alloc_size);
            memset(buffer, CMP_VAL, alloc_size);

            if( NULL == buffer )
                ret = STATUS_NOT_ENOUGH_MEMORY;
        }

        DVP_S32 offset = 0, *ind, zeroVal = 0;

        if( use_single_image ) {
            ind = &zeroVal;
        }
        else {
            ind = &count;
        }

        if( STATUS_SUCCESS == ret )
        {
            for( count = 0; count < num_imgs_max; count++ )
            {
                if( pImg_Ser[*ind] && buffer )
                {
                    offset += DVP_Image_Serialize( pImg_Ser[*ind], &buffer[offset], (alloc_size - offset));
                }
            }

            if( !strcmp(testVariant, SERIALIZE) && buffer )
            {
                DVP_U32 len = alloc_size - offset;
                DVP_U08 *mem = malloc(len);
                if( mem )
                {
                    memset(mem, CMP_VAL, len);
                    if ( memcmp(&buffer[offset], mem, len ) != 0)
                         ret = STATUS_FAILURE;
                    free(mem);
                    mem = NULL;
                }
            }
            else if ( !strcmp(testVariant, UNSERIALIZE) /*&& buffer*/ )
            {
                DVP_Image_t *pImg_Unser = malloc(sizeof(DVP_Image_t));

                if( pImg_Unser )
                {
                    DVP_Image_Init(pImg_Unser, width, height, FOURCC_NV12);
                    if( DVP_Image_Alloc(dvp, pImg_Unser, DVP_MTYPE_DEFAULT) )
                    {
                        size_t offset = 0;
                        for( count = 0; count < num_imgs_max; count++ )
                        {
                            offset = DVP_Image_Unserialize(pImg_Unser, &buffer[offset], (alloc_size - offset));
                            if( DVP_FALSE == DVP_Image_Equal(pImg_Ser[*ind],pImg_Unser) )
                            {
                                ret = STATUS_FAILURE;
                            }
                        }
                        DVP_Image_Free(dvp, pImg_Unser);
                    }
                    DVP_Image_Deinit(pImg_Unser);
                    free(pImg_Unser);
                }
                else
                    ret = STATUS_NOT_ENOUGH_MEMORY;
            }
        }

        for( count = 0; count < num_imgs_max; count++ )
        {
            if( pImg_Ser[*ind] )
            {
                if( DVP_Image_Free(dvp, pImg_Ser[*ind]) )
                {
                    DVP_Image_Deinit(pImg_Ser[*ind]);
                    free( pImg_Ser[*ind] );
                    pImg_Ser[*ind] = NULL;
                }
            }
        }

        if( buffer )
        {
            free(buffer);
            buffer = NULL;
        }
        DVP_KernelGraph_Deinit(dvp);
    }

    return ret;
}

status_e dvp_serialize_test(void)
{
    return dvp_image_serialization(SERIALIZE );
}


status_e dvp_unserialize_test(void)
{
    return dvp_image_serialization(UNSERIALIZE);
}

status_e dvp_image_size_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    DVP_Image_t *img = NULL;
    DVP_U32 size = 0;
    if( dvp )
    {
        img = malloc(sizeof(DVP_Image_t));
        if( img )
        {
            DVP_Image_Init(img, width, height, FOURCC_YV12);
            if( DVP_Image_Alloc(dvp, img, DVP_MTYPE_DEFAULT) )
            {
                size = DVP_Image_Size(img);
                if( size == img->numBytes )
                    status = STATUS_SUCCESS;
                else
                    DVP_PRINT( DVP_ZONE_ALWAYS, "Expected Image Size is %d, but found is %d.\n",
                               img->numBytes, size);

                DVP_Image_Free(dvp, img);
                DVP_Image_Deinit(img);
                free(img);
            }
        }

        DVP_KernelGraph_Deinit(dvp);
    }
    return status;
}



status_e dvp_buffer_init_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Buffer_t *pBuffer = malloc(sizeof(DVP_Buffer_t));
    if( pBuffer )
    {
        DVP_U32 elSize = 16, elNum = 10;
        DVP_Buffer_Init( pBuffer, elSize, elNum );
        if( pBuffer->elemSize == elSize && pBuffer->numBytes == (elSize * elNum))
        {
            status = STATUS_SUCCESS;
        }
        DVP_Buffer_Deinit( pBuffer );
        free(pBuffer);
        pBuffer = NULL;
    }

    return status;
}

status_e dvp_buffer_allocation_test(const char *memOperation)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();

    if( dvp )
    {
        DVP_U32 elSize = 16, elNum = 10;
        DVP_Buffer_t *pBuffer = malloc(sizeof(DVP_Buffer_t));
        if( pBuffer )
        {
            DVP_Buffer_Init( pBuffer, elSize, elNum );
            if ( DVP_Buffer_Alloc(dvp, pBuffer, DVP_MTYPE_DEFAULT) )
            {
                if( !strcmp(ALLOC, memOperation ))
                    status = STATUS_SUCCESS;

                if( DVP_Buffer_Free(dvp, pBuffer) )
                {
                    if( !strcmp(FREE, memOperation ))
                        status = STATUS_SUCCESS;
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ALWAYS, "DVP_Buffer_Free() failed ! \n");
                }
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ALWAYS, "DVP_Buffer_Alloc() failed ! \n");
            }
            DVP_Buffer_Deinit( pBuffer );
            free(pBuffer);
            pBuffer = NULL;
        }
        DVP_KernelGraph_Deinit(dvp);
    }

    return status;
}

status_e dvp_buffer_alloc_test(void)
{
    return dvp_buffer_allocation_test(ALLOC);
}

status_e dvp_buffer_free_test(void)
{
    return dvp_buffer_allocation_test(FREE);
}

status_e dvp_memory_import_test(void)
{
    DVP_Handle mem_importer = DVP_MemImporter_Create();
    if( mem_importer )
    {
        DVP_MemImporter_Free(mem_importer);
        return STATUS_SUCCESS;
    }
    else
    {
        return STATUS_FAILURE;
    }
}


status_e dvp_buffer_share_test(void)
{
    status_e stat = STATUS_FAILURE;
    DVP_S08 mem[width*height];
    DVP_Buffer_t *pBuffer = malloc(sizeof(DVP_Buffer_t));
    DVP_Handle dvp = DVP_KernelGraph_Init();

    if( dvp )
    {
        if( pBuffer )
        {
            memset(mem, 0x57, sizeof(mem));

            DVP_Buffer_Init(pBuffer, sizeof(DVP_Image_t), 32);
            pBuffer->memType = DVP_MTYPE_DEFAULT;

            if( DVP_TRUE == DVP_Buffer_Alloc(dvp, pBuffer, (DVP_MemType_e)(pBuffer->memType)) )
            {
//                DVP_Image_Fill(pBuffer, mem, sizeof(mem));

                DVP_S32 fd[1] = { 0 };
                if( DVP_TRUE == DVP_Buffer_Share( dvp, pBuffer, fd ) )
                {
                    stat = STATUS_SUCCESS;
                }
                DVP_Buffer_Free(dvp, pBuffer);
            }
            DVP_Buffer_Deinit(pBuffer);
            free(pBuffer);
        }
        DVP_KernelGraph_Deinit(dvp);
    }

    return stat;
}

status_e dvp_buffer_import_test(void)
{
    status_e status = STATUS_FAILURE;
#if 0
    DVP_S08 mem[width*height];
#endif

    DVP_Buffer_t *pBuffer = malloc(sizeof(DVP_Buffer_t));

    DVP_Handle dvp_importer = DVP_MemImporter_Create();
    DVP_Handle dvp_allocator = DVP_KernelGraph_Init();

    if( dvp_importer && dvp_allocator )
    {
        if( pBuffer )
        {

            DVP_Buffer_Init(pBuffer, sizeof(DVP_Image_t), 32 );
            pBuffer->memType = DVP_MTYPE_DEFAULT;

            if( DVP_TRUE == DVP_Buffer_Alloc(dvp_allocator, pBuffer, (DVP_MemType_e)(pBuffer->memType)) )
            {
#if 0
    memset(mem, 0x57, sizeof(mem));
    DVP_Buffer_Fill(pBuffer, mem, sizeof(mem));
#endif
                DVP_S32 fds[1] = { 0 };
                if( DVP_TRUE == DVP_Buffer_Share( dvp_allocator, pBuffer, fds ) )
                {
                    DVP_VALUE hdls[1] = { 0 };
                    if( DVP_TRUE == DVP_Buffer_Import(dvp_importer, pBuffer, fds[0], hdls) )
                    {
                        /// dostatachno li e ili da se napravi neshto s taq kartinka ???
                        /// dali ako iskam da se obraboti, ne trqbva nov dvp_handle, napraven ot graph init????
                        DVP_PrintBuffer(DVP_ZONE_ALWAYS, pBuffer);
                        status = STATUS_SUCCESS;
                        /// dali ne e po-dobre da sa v otdelni funkcii (import i import_free) ???
                        if( DVP_TRUE == DVP_Buffer_Free_Import(dvp_importer, pBuffer, hdls[0]) )
                        {
                            ;
                        }
                        else
                        {
                            DVP_PRINT(DVP_ZONE_ALWAYS, "DVP_Buffer_Free_Import() failed !!!\n");
                        }
                    }
                    else
                    {
                        DVP_PRINT(DVP_ZONE_ALWAYS, "DVP_Buffer_Import() failed !!!\n");
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ALWAYS, "DVP_Buffer_Share() failed !!!\n");
                }

                DVP_Buffer_Free(dvp_allocator, pBuffer);
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ALWAYS, "Failed to allocate DVP_Buffer!!!\n");
            }

            DVP_Buffer_Deinit(pBuffer);

            free(pBuffer);
        }
        DVP_MemImporter_Free(dvp_importer);
        DVP_KernelGraph_Deinit(dvp_allocator);
    }

    return status;
}

status_e dvp_imageshift_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Handle dvp = DVP_KernelGraph_Init();
    if (dvp)
    {
        DVP_U32 numNodes = 1;
        DVP_U32 numSections = 1;
        dvp_image_shift_t shift;
        DVP_KernelNode_t *nodes = DVP_KernelNode_Alloc(dvp, numNodes);
        if (nodes)
        {
            DVP_KernelGraph_t *graph = DVP_KernelGraph_Alloc(dvp, numSections);
            if (graph)
            {
                DVP_Error_e err = DVP_SUCCESS;
                err = DVP_KernelGraphSection_Init(dvp, graph, 0, nodes, numNodes);
                if (err == DVP_SUCCESS)
                {
                    DVP_Image_t images[3];
                    DVP_ImageConvolution_t *pT = dvp_knode_to(&nodes[0], DVP_ImageConvolution_t);
                    DVP_S08 mask[] = {1,2,1,
                                      2,4,2,
                                      1,2,1};

                    DVP_Image_Init(&images[0], 640, 480, FOURCC_Y800);
                    DVP_Image_Init(&images[1], 640, 480, FOURCC_Y800);
                    DVP_Image_Init(&images[2], 3, 3, FOURCC_Y800);

                    DVP_Image_Alloc(dvp, &images[0], DVP_MTYPE_DEFAULT);
                    DVP_Image_Alloc(dvp, &images[1], DVP_MTYPE_DEFAULT);
                    DVP_Image_Alloc(dvp, &images[2], DVP_MTYPE_DEFAULT);

                    DVP_Image_Fill(&images[2], mask, sizeof(mask));

#if defined(DVP_USE_VRUN)
                    nodes[0].header.kernel = DVP_KN_VRUN_CONV_MxN;
                    nodes[0].header.affinity = DVP_CORE_SIMCOP;
#endif
                    DVP_Image_Dup(&pT->input, &images[0]);
                    DVP_Image_Dup(&pT->output, &images[1]);
                    DVP_Image_Dup(&pT->mask, &images[2]);
                    pT->shiftMask = 4;

                    if (DVP_KernelGraph_Verify(dvp, graph))
                    {
                        memset(&shift, 0, sizeof(shift));
                        DVP_KernelGraph_ImageShiftAccum(dvp, &nodes[0], &shift);

                        DVP_PRINT(DVP_ZONE_ALWAYS, "ImageShift = {%d, %d}\n", shift.centerShiftHorz, shift.centerShiftVert);

                        if (shift.centerShiftHorz == -(DVP_S32)images[2].width/2 &&
                            shift.centerShiftVert == -(DVP_S32)images[2].height/2)
                            status = STATUS_SUCCESS;
                    }
                }
                DVP_KernelGraph_Free(dvp, graph);
                graph = NULL;
            }
            DVP_KernelNode_Free(dvp, nodes, numNodes);
            nodes = NULL;
        }
        DVP_KernelGraph_Deinit(dvp);
    }
    return status;
}


void create_dvp_image(DVP_Handle *dvp, DVP_Image_t **pImage, DVP_U32 img_width, DVP_U32 img_height, fourcc_t fourcc)
{
    *dvp = DVP_KernelGraph_Init();
    if( *dvp )
    {
        *pImage = malloc(sizeof(DVP_Image_t));
        if( *pImage )
        {
            DVP_Image_Init(*pImage, img_width, img_height, fourcc);
            if( DVP_FALSE == DVP_Image_Alloc( *dvp, *pImage, DVP_MTYPE_DEFAULT ) )
            {
                DVP_Image_Deinit(*pImage);
                free(*pImage);
                *pImage = NULL;

                DVP_KernelGraph_Deinit(*dvp);
                *dvp = 0;
            }
        }
    }
}

void destroy_dvp_image(DVP_Handle *dvp, DVP_Image_t *pImage )
{
    if( *dvp )
    {
        if( pImage && DVP_TRUE == DVP_Image_Free( *dvp, pImage ) )
        {
            DVP_Image_Deinit(pImage);
            free(pImage);
            pImage = NULL;

            DVP_KernelGraph_Deinit(*dvp);
            *dvp = 0;
        }
    }
}

status_e dvp_image_width_div_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;
    DVP_U32 plane = -1;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        DVP_U32 div = 1;
        for ( plane = 0; plane < pImage->planes; plane++ )
        {
            switch (pImage->color)
            {
                case FOURCC_IYUV:
                case FOURCC_YV12:
                case FOURCC_NV12:
                case FOURCC_NV21:
                case FOURCC_YU16:
                case FOURCC_YV16:
                    div = 2;
                    break;
                case FOURCC_YUV9:
                case FOURCC_YVU9:
                    div = 4;
                    break;
                default:
                    div = 1;
                    break;
            }
            if( div == DVP_Image_WidthDiv(pImage, plane) ) {
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_FAILURE;
            }
        }
        destroy_dvp_image(&dvp, pImage);
    }
    return status;
}

status_e dvp_image_height_div_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;
    DVP_U32 plane = -1;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        for ( plane = 0; plane < pImage->planes; plane++ )
        {
            DVP_U32 div = 1;
            if (plane > 0)
            {
                switch (pImage->color)
                {
                    case FOURCC_IYUV:
                    case FOURCC_YV12:
                    case FOURCC_NV12:
                    case FOURCC_NV21:
                        div = 2;
                        break;
                    case FOURCC_YUV9:
                    case FOURCC_YVU9:
                        div = 4;
                        break;
                    case FOURCC_YU16:
                    case FOURCC_YV16:
                    default:
                        div = 1;
                        break;
                }
            }
            else if (pImage->color == FOURCC_BIN1){
                div = 8;
            }

            if( div == DVP_Image_HeightDiv(pImage, plane) ) {
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_FAILURE;
                break;
            }
        }
        destroy_dvp_image(&dvp, pImage);
    }
    return status;
}

DVP_U32 dvp_test_image_width_step(DVP_Image_t *pImage, DVP_U32 plane)
{
    DVP_U32 step = 1;
    if (pImage && plane < pImage->planes && plane > 0)
    {
        switch (pImage->color)
        {
            case FOURCC_NV12:
            case FOURCC_NV21:
                step = 2;
                break;
        }
    }
    return step;
}

status_e dvp_image_line_range_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;
    DVP_U32 plane = -1;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        for ( plane = 0; plane < pImage->planes; plane++ )
        {
            DVP_U32 lineRange = abs(pImage->y_stride);
            DVP_U32 priLineSize = 0, secLineSize = 0;
            if (plane > 0)
            {
                DVP_U32 xdiv = DVP_Image_WidthDiv(pImage, plane);
                DVP_U32 xstep = dvp_test_image_width_step(pImage, plane);

                priLineSize = (pImage->x_stride * dvp_test_image_width_step(pImage, 0)) *
                              (pImage->bufWidth / DVP_Image_WidthDiv(pImage, 0));
                secLineSize = (pImage->x_stride * xstep) *
                              (pImage->bufWidth / xdiv);
                if (lineRange == priLineSize)
                    lineRange = secLineSize;
            }

             if ( lineRange == DVP_Image_LineRange(pImage, plane) ) {
                 status = STATUS_SUCCESS;
             }
             else {
                 status = STATUS_FAILURE;
                 break;
             }
        }

        destroy_dvp_image(&dvp, pImage);
    }

    return status;
}

status_e dvp_image_plane_range_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;
    DVP_U32 plane = -1;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        for ( plane = 0; plane < pImage->planes; plane++ )
        {
            DVP_U32 lineRange = 0;
            DVP_U32 planeRange = 0;

            if (pImage != NULL && plane < pImage->planes)
            {
                DVP_U32 ydiv = DVP_Image_HeightDiv(pImage, plane);
                lineRange = DVP_Image_LineRange(pImage, plane);
                planeRange = lineRange * pImage->bufHeight/ydiv;
            }

            if ( planeRange == DVP_Image_PlaneRange(pImage, plane ) ) {
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_FAILURE;
                break;
            }
        }

        destroy_dvp_image(&dvp, pImage);
    }
    return status;
}

status_e dvp_image_range_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        DVP_U32 plane,expRange = 0;
        for (plane = 0; plane < pImage->planes; plane++)
            expRange += DVP_Image_PlaneSize(pImage, plane);

        if ( expRange == DVP_Image_Range(pImage) ) {
            status = STATUS_SUCCESS;
        }
        destroy_dvp_image(&dvp, pImage);
    }

    return status;
}

status_e dvp_image_addressing_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;
    DVP_U32 plane = -1;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        for ( plane = 0; plane < pImage->planes; plane++ )
        {
            DVP_U08 *ptr = pImage->pBuffer[plane];
            DVP_U32 i = DVP_Image_Offset(pImage, (width/4), (height/4), plane);

            if ( &ptr[i] == DVP_Image_Addressing(pImage, (width/4), (height/4), plane) ) {
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_FAILURE;
                break;
            }
        }

        destroy_dvp_image(&dvp, pImage);
    }
    return status;
}

status_e dvp_image_offset_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;
    DVP_U32 plane = -1;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        for ( plane = 0; plane < pImage->planes; plane++ )
        {
            DVP_U32 expRes = 0, x, y;
            x = width /2;
            y = height / 2;
            if ( x < pImage->bufWidth && y < pImage->bufHeight )
            {
                DVP_U32 xstep = dvp_test_image_width_step(pImage, plane);
                DVP_U32 xdiv = DVP_Image_WidthDiv(pImage, plane);
                DVP_U32 ydiv = DVP_Image_HeightDiv(pImage, plane);
                DVP_U32 ystr = DVP_Image_LineRange(pImage, plane);

                expRes = ((y/ydiv) * ystr) + ((x/xdiv) * (pImage->x_stride*xstep));
            }

            if ( expRes == DVP_Image_Offset(pImage, x, y, plane ) ) {
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_FAILURE;
                break;
            }
        }
        destroy_dvp_image(&dvp, pImage);
    }
    return status;
}

status_e dvp_image_equal_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage[2] = { NULL, NULL };
    DVP_Handle dvp = 0;

    create_dvp_image(&dvp, &pImage[0], width, height, FOURCC_NV12);

    pImage[1] = malloc(sizeof(DVP_Image_t));
    DVP_Image_Init(pImage[1], width, height, FOURCC_NV12);

    if( dvp && pImage[0] && DVP_Image_Alloc(dvp, pImage[1], DVP_MTYPE_DEFAULT) )
    {
        DVP_U32 bfLen = (height * width * 3u) / 2u ;
        DVP_S08 ptr[bfLen];
        memset(ptr, 0x57, bfLen);

        DVP_Image_Fill(pImage[0], ptr, bfLen );
        DVP_Image_Fill(pImage[1], ptr, bfLen );

        if ( DVP_Image_Equal(pImage[0], pImage[1]) ) {
            status = STATUS_SUCCESS;
        }

        DVP_Image_Free( dvp, pImage[1] );
        DVP_Image_Deinit(pImage[1]);
        free(pImage[1]);
        pImage[1] = NULL;

        destroy_dvp_image(&dvp, pImage[0] );
    }
    return status;
}

status_e dvp_image_patch_line_size_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;
    DVP_U32 plane = -1;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        DVP_Image_SetPatch(pImage, (width/4), (height/4), (width/2), (height/2) );

        for ( plane = 0; plane < pImage->planes; plane++ )
        {
            DVP_U32 exp_lineSize = 0;
            if (pImage != NULL && plane < pImage->planes)
            {
                DVP_U32 xstep = dvp_test_image_width_step(pImage, plane);
                DVP_U32 xdiv = DVP_Image_WidthDiv(pImage, plane);

                exp_lineSize = (pImage->x_stride * xstep) * (pImage->width / xdiv);
            }

            if ( exp_lineSize == DVP_Image_PatchLineSize(pImage, plane) ) {
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_FAILURE;
                break;
            }
        }

        destroy_dvp_image(&dvp, pImage);
    }
    return status;
}

status_e dvp_image_patch_plane_size_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;
    DVP_U32 plane = -1;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        DVP_Image_SetPatch(pImage, (width/4), (height/4), (width/2), (height/2) );

        for ( plane = 0; plane < pImage->planes; plane++ )
        {
            DVP_U32 lineSize = 0;
            DVP_U32 exp_planeSize = 0;

            if (pImage != NULL && plane < pImage->planes)
            {
                DVP_U32 ydiv = DVP_Image_HeightDiv(pImage, plane);

                lineSize = DVP_Image_PatchLineSize(pImage, plane);
                exp_planeSize = lineSize * pImage->height/ydiv;
            }

            if( exp_planeSize == DVP_Image_PatchPlaneSize(pImage, plane) ) {
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_FAILURE;
                break;
            }
        }
        destroy_dvp_image(&dvp, pImage);
    }
    return status;
}

status_e dvp_image_patch_size_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        DVP_Image_SetPatch(pImage, (width/4), (height/4), (width/2), (height/2) );

        DVP_U32 p,size = 0;
        if (pImage != NULL) {
            for (p = 0; p < pImage->planes; p++) {
                size += DVP_Image_PatchPlaneSize(pImage, p);
            }
        }

        if( size == DVP_Image_PatchSize(pImage) ) {
            status = STATUS_SUCCESS;
        }
        else {
            status = STATUS_FAILURE;
        }
        destroy_dvp_image(&dvp, pImage);
    }
    return status;
}

status_e dvp_image_patch_addressing_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;
    DVP_U32 plane = -1;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        DVP_U32 x_start , y_start;
        x_start = width/4;
        y_start = height/4;

        DVP_Image_SetPatch(pImage, x_start, y_start, (width/2), (height/2) );

        for ( plane = 0; plane < pImage->planes ; plane++ )
        {
            DVP_U08 *ptr = NULL;
            DVP_U32 i = DVP_Image_PatchOffset(pImage, x_start, y_start, plane);
            if (pImage != NULL && plane < pImage->planes) {
                ptr = pImage->pData[plane];
            }

            if ( &ptr[i] == DVP_Image_PatchAddressing(pImage, x_start, y_start, plane) ) {
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_FAILURE;
                break;
            }
        }

        destroy_dvp_image(&dvp, pImage);
    }
    return status;
}

status_e dvp_image_patch_offset_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;
    DVP_U32 plane = -1;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        for ( plane = 0; plane < pImage->planes; plane++ )
        {
            DVP_U32 x_start , y_start;
            x_start = width/4;
            y_start = height/4;

            DVP_Image_SetPatch(pImage, x_start, y_start, (width/2), (height/2) );
            DVP_U32 expValue = 0u;
            if (pImage != NULL && plane < pImage->planes && x_start < pImage->width && y_start < pImage->height )
            {
                DVP_U32 xstep = dvp_test_image_width_step(pImage, plane);
                DVP_U32 xdiv = DVP_Image_WidthDiv(pImage, plane);
                DVP_U32 ydiv = DVP_Image_HeightDiv(pImage, plane);
                DVP_U32 ystr = DVP_Image_LineRange(pImage, plane);

                expValue = ((y_start/ydiv) * ystr) + ((x_start/xdiv) * (pImage->x_stride*xstep));
            }

            if ( expValue == DVP_Image_PatchOffset(pImage, x_start, y_start, plane) ) {
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_FAILURE;
                break;
            }
        }
        destroy_dvp_image(&dvp, pImage);
    }
    return status;
}

status_e dvp_image_set_patch_test(void)
{
    status_e status = STATUS_FAILURE;
    DVP_Image_t *pImage = NULL;
    DVP_Handle dvp = 0;

    create_dvp_image(&dvp, &pImage, width, height, FOURCC_NV12);
    if( dvp && pImage )
    {
        DVP_U32 x_start , y_start, patch_width, patch_height;
        x_start = width/4;
        y_start = height/4 ;
        patch_width = width/2;
        patch_height = height/2;

        if( DVP_Image_SetPatch(pImage, x_start, y_start, patch_width, patch_height ) ) {
            status = STATUS_SUCCESS;
        }

        destroy_dvp_image(&dvp, pImage );
    }
    return status;
}

status_e dvp_image_patch_equal_test(void)
{
    status_e status = STATUS_FAILURE;

    DVP_Image_t *pImage[2] = { NULL, NULL };
    DVP_Handle dvp = 0;

    create_dvp_image(&dvp, &pImage[0], width, height, FOURCC_NV12);

    pImage[1] = (DVP_Image_t*)malloc(sizeof(DVP_Image_t));
    DVP_Image_Init(pImage[1], width, height, FOURCC_NV12);

    if( dvp && pImage[0] && DVP_Image_Alloc(dvp, pImage[1], DVP_MTYPE_DEFAULT) )
    {
        DVP_U32 bfLen = (height * width * 3u) / 2u ;
        DVP_S08 ptr[bfLen];
        memset(ptr, 0x57, sizeof(ptr));

        DVP_Image_Fill(pImage[0], ptr, sizeof(ptr) );
        DVP_Image_Fill(pImage[1], ptr, sizeof(ptr) );

        DVP_Image_SetPatch( pImage[0], (width/4), (height/4), (width/2), (height/2) );
        DVP_Image_SetPatch( pImage[1], (width/4), (height/4), (width/2), (height/2) );

        if ( DVP_Image_PatchEqual(pImage[0], pImage[1]) ) {
            status = STATUS_SUCCESS;
        }

        DVP_Image_Free(dvp, pImage[1]);
        DVP_Image_Deinit(pImage[1]);
        free(pImage[1]);
        pImage[1] = NULL;

        destroy_dvp_image(&dvp, pImage[0] );
    }

    return status;
}


/*! \brief Local Unit Test Function Pointer */
typedef status_e (*dvp_unittest_f)(void);

/*! \brief Unit test meta-data structure used to track result of each function */
typedef struct _dvp_unittest_t {
    status_e status;
    char name[MAX_PATH];
    dvp_unittest_f function;
} dvp_unittest_t;

dvp_unittest_t unittests[] = {
    {STATUS_FAILURE, "Framework: Context", dvp_context_test},
    {STATUS_FAILURE, "Framework: Query System", dvp_info_test},
    {STATUS_FAILURE, "Framework: CPU Nop Test", dvp_cpu_nop_test},
    {STATUS_FAILURE, "Framework: SERIAL Nop Test", dvp_serial_nop_test},
    {STATUS_FAILURE, "Framework: PARALLEL Nop Test", dvp_parallel_nop_test},
    {STATUS_FAILURE, "Framework: SERIAL Copy Test", dvp_copy_test},
    {STATUS_FAILURE, "Framework: CUSTOM Copy Test", dvp_custom_copy_test},
#if defined(DVP_USE_YUV)
    {STATUS_FAILURE, "Framework: PARALLEL CC Test", dvp_split_cc_test},
#endif
    {STATUS_FAILURE, "Framework: ImageShift", dvp_imageshift_test},
    {STATUS_FAILURE, "Framework: Core Capacity Test", dvp_capacity_test },
    {STATUS_FAILURE, "Framemork: Kernel Node Alloc Test", dvp_kern_node_alloc_test},
    {STATUS_FAILURE, "Framemork: Kernel Node Free Test", dvp_kern_node_free_test},
    {STATUS_FAILURE, "Framemork: Query Kernel Test", dvp_query_kernel_test},
    {STATUS_FAILURE, "Framework: Memory Import Test", dvp_memory_import_test},
    {STATUS_FAILURE, "Framework: Buffer Image Init Test", dvp_image_init_test},
    {STATUS_FAILURE, "Framework: Buffer Image Alloc Test", dvp_image_alloc_test},
    {STATUS_FAILURE, "Framework: Buffer Image Free Test", dvp_image_free_test},
    {STATUS_FAILURE, "Framework: Image Share Test", dvp_image_share_test},
    {STATUS_FAILURE, "Framework: Image Importer Test", dvp_image_import_test},
    {STATUS_FAILURE, "Framework: Image Importer Free Test", dvp_image_import_free_test},
    {STATUS_FAILURE, "Framework: Image Copy Test", dvp_img_copy_test},
    {STATUS_FAILURE, "Framework: Image Serialize Test", dvp_serialize_test},
    {STATUS_FAILURE, "Framework: Image Unerialize Test", dvp_unserialize_test},
    {STATUS_FAILURE, "Framework: Get image Size Test", dvp_image_size_test},
    {STATUS_FAILURE, "Framework: Buffer Init Test", dvp_buffer_init_test},
    {STATUS_FAILURE, "Framework: Buffer Alloc Test", dvp_buffer_alloc_test},
    {STATUS_FAILURE, "Framework: Buffer Free Test", dvp_buffer_free_test},
    {STATUS_FAILURE, "Framework: Buffer Share Test", dvp_buffer_share_test},
    {STATUS_FAILURE, "Framework: Buffer Importer Test", dvp_buffer_import_test},
    {STATUS_FAILURE, "Framework: Image Height Div Test", dvp_image_height_div_test},
    {STATUS_FAILURE, "Framework: Image Width Div Test", dvp_image_width_div_test},
    {STATUS_FAILURE, "Framework: Image Line Range Test", dvp_image_line_range_test},
    {STATUS_FAILURE, "Framework: Image Plane Range Test", dvp_image_plane_range_test},
    {STATUS_FAILURE, "Framework: Image Range Test", dvp_image_range_test},
    {STATUS_FAILURE, "Framework: Image Addressing Test", dvp_image_addressing_test},
    {STATUS_FAILURE, "Framework: Image Offset Test", dvp_image_offset_test},
    {STATUS_FAILURE, "Framework: Image Equal Test", dvp_image_equal_test},
    {STATUS_FAILURE, "Framework: Image Patch Line Size Test", dvp_image_patch_line_size_test},
    {STATUS_FAILURE, "Framework: Image Patch Plane Size Test", dvp_image_patch_plane_size_test},
    {STATUS_FAILURE, "Framework: Image Patch Size Test", dvp_image_patch_size_test},
    {STATUS_FAILURE, "Framework: Image Patch Addressing Test", dvp_image_patch_addressing_test},
    {STATUS_FAILURE, "Framework: Image Patch Offset Test", dvp_image_patch_offset_test},
    {STATUS_FAILURE, "Framework: Image Set Patch Test", dvp_image_set_patch_test},
    {STATUS_FAILURE, "Framework: Image Patch Equal Test", dvp_image_patch_equal_test},

};

int main(int argc, char *argv[])
{
    uint32_t i,j;
    int err = 0;
    int passed = 0;
    iterations = 1;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    option_process(argc, argv, opts, dimof(opts));
    for (i = 0; i < dimof(unittests); i++)
    {
        for (j = 0; j < iterations; j++)
        {
            unittests[i].status = unittests[i].function();
            if (unittests[i].status == STATUS_SUCCESS)
                passed++;
        }
        DVP_PRINT(DVP_ZONE_ALWAYS, "[%s] %s\n", (unittests[i].status == STATUS_SUCCESS?"PASSED":"FAILED"), unittests[i].name);
    }

    DVP_PRINT(DVP_ZONE_ALWAYS, "Passed %d out of %u\n", passed, dimof(unittests));

    return err;
}

