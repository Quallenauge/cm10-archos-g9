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

#ifndef _DVP_KGRAPH_H_
#define _DVP_KGRAPH_H_


/*!
 * \file dvp_api.h
 * \brief The Distributed Vision Processing API Specification.
 * \author Erik Rainey
 */

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief This function creates an instance of DVP which can process a kernel graph.
 * \note This function must be called before any other call in DVP.
 * \ingroup group_system
 */
DVP_Handle DVP_KernelGraph_Init();

/*!
 * \brief This function creates an instance of DVP which can only import and free imported memory.
 * \ingroup group_system
 */
DVP_Handle DVP_MemImporter_Create();

/*!
 * \brief This function destroys an existing DVP instance.
 * \param [in] The handle to the DVP instance.
 * \pre DVP_KernelGraph_Init
 * \ingroup group_system
 */
void DVP_KernelGraph_Deinit(DVP_Handle handle);

/*!
 * \brief This function destroys an existing mem handler DVP instance.
 * \param [in] handle The hande to the DVP instance.
 * \ingroup group_system
 */
void DVP_MemImporter_Free(DVP_Handle handle);

/*!
 * \brief This function creates a graph with the requested number of sections.
 * \param [in] handle The handle to the DVP context.
 * \param [in] numSections The number of sections needed.
 * \return Returns a Kernel Graph pointer.
 * \retval NULL Could not allocate a graph.
 * \ingroup group_graphs
 */
DVP_KernelGraph_t *DVP_KernelGraph_Alloc(DVP_Handle handle, DVP_U32 numSections);

/*!
 * \brief This function frees the sections, order and graph.
 * \note Nodes will not be freed! Your graph pointer will need to be set to NULL
 * after this function.
 * \param [in] handle The handle to the DVP context.
 * \param [in] graph The pointer to the graph to be freed.
 * \ingroup group_graphs
 */
void DVP_KernelGraph_Free(DVP_Handle handle, DVP_KernelGraph_t *graph);

/*!
 * \brief This function initializes the desired section to execute the nodes
 * indicated.
 * \param [in] handle The handle to the DVP context.
 * \param [in] graph The pointer to the graph to modify.
 * \param [in] sectionIndex The index of the section to modify
 * \param [in] pNodes The pointer to the first node for this section to execute.
 * \param [in] numNodes The number of nodes from the indicated node to assign to
 * the section.
 * \ingroup group_sections
 */
DVP_Error_e DVP_KernelGraphSection_Init(DVP_Handle handle,
                                      DVP_KernelGraph_t *graph,
                                      DVP_U32 sectionIndex,
                                      DVP_KernelNode_t *pNodes,
                                      DVP_U32 numNodes);

/*!
 * \brief The typedef for callbacks from completed sections.
 * \param [in] cookie Private pointer supplied to \ref vlProcessGraph.
 * \param [in] graph The pointer to the graph.
 * \param [in] sectionIndex The index of which section completed.
 * \param [in] numNodesExecuted The number of nodes in the section which completed.
 * \ingroup group_sections
 */
typedef void (*DVP_SectionComplete_f)(void *cookie, DVP_KernelGraph_t *graph, DVP_U32 sectionIndex, DVP_U32 numNodesExecuted);

/*!
 * \brief This function verifies the correctness of a DVP graph.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in] graph The pointer to a Kernel Graph structure.
 * \return Returns a boolean to indicate if the graph is correct.
 * \note Each Kernel Node in the graph will be set to a specific error value.
 * \post \ref DVP_KernelGraph_Process
 * \ingroup group_graphs
 */
DVP_BOOL DVP_KernelGraph_Verify(DVP_Handle handle, DVP_KernelGraph_t *graph);

/*!
 * \brief This function processes a \ref DVP_KernelGraph_t and will inform
 * the client after each section of nodes executed in that graph,
 * successful or not.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in] graph The pointer to a Kernel Graph structure.
 * \param [in] cookie A user supplied pointer to pass to the callback
 * \param [in] callback The function to call after each graph completes.
 * \note In parallel mode, the callback is mutex protected.
 * \pre \ref DVP_KernelGraph_Process must be called at least once before process
 * or an overhead penalty of checking the graph will be incurred.
 * \ingroup group_graphs
 */
DVP_U32 DVP_KernelGraph_Process(DVP_Handle handle,
                                DVP_KernelGraph_t *graph,
                                void *cookie,
                                DVP_SectionComplete_f callback);

/*!
 * \brief This function clears and initializes the DVP_Perf_t structure.
 * \ingroup group_performance
 * \param [out] perf The performance structure to clear.
 */
void DVP_Perf_Clear(DVP_Perf_t *perf);

/*!
 * \brief Initializes the performance data in each node. All prexisting performance data is lost.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in] pNodes The array of nodes to clear.
 * \param [in] numNodes The number of nodes in pNodes
 * \ingroup group_performance
 */
void DVP_PerformanceClear(DVP_Handle handle, DVP_KernelNode_t * pNodes, DVP_U32 numNodes);

/*!
 * \brief Captures the start time.
 * \param [in] pPerf The pointer to the performance structure
 * \pre \ref DVP_Perf_Clear
 * \ingroup group_performance
 */
void DVP_PerformanceStart(DVP_Perf_t *pPerf);

/*!
 * \brief Captures the stop time and updates the other performance variables.
 * \param [in] pPerf The pointer to the performance structure
 * \pre \ref DVP_PerformanceStart
 * \ingroup group_performance
 */
void DVP_PerformanceStop(DVP_Perf_t *pPerf);

/*!
 * \brief Prints the Performance information in the structure with a custom prefix string.
 * \param [in] pPerf The pointer to the performance structure
 * \param [in] prefix A short character series to print before the performance information.
 * \pre \ref DVP_PerformanceStop
 * \ingroup group_performance
 */
void DVP_PerformancePrint(DVP_Perf_t *pPerf, const char *prefix);

/*!
 * \brief This function calls printf to list the performance metrics of each node in
 * a format which can be cut and pasted into a CSV file.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in] pNodes The array of nodes to print the performance information from.
 * \param [in] numNodes The number of nodes which \ref nodes refers to.
 * \ingroup group_performance
 */
void DVP_PrintPerformanceCSV(DVP_Handle handle, DVP_KernelNode_t * pNodes, DVP_U32 numNodes);

/*!
 * \brief This function call printf to list the performance metrics of each section and the overall
 * graph performance.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in] pSection The section to print performance from.
 * \ingroup group_performance
 */
void DVP_PrintPerformanceSection(DVP_Handle handle, DVP_KernelGraphSection_t *pSection);

/*!
 * \brief This function call printf to list the performance metrics of each section and the overall
 * graph performance.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in] pGraph The graph to print performance from.
 * \ingroup group_performance
 */
void DVP_PrintPerformanceGraph(DVP_Handle handle, DVP_KernelGraph_t *pGraph);

/*!
 * \brief This prints ou the members of the Kernel Node structure.
 * \param [in] zone The debugging zone to place the printout in.
 * \param [in] node The pointer to the node to print.
 * \ingroup group_nodes
 */
void DVP_PrintNode(DVP_U32 zone, DVP_KernelNode_t *node);

/*!
 * \brief This prints out the members of the image structure in the given zone.
 * \param [in] zone The debugging zone to place the printout in.
 * \param [in] img The pointer to the DVP_Image_t to print.
 * \ingroup group_image
 */
void DVP_PrintImage(DVP_U32 zone, DVP_Image_t *img);

/*!
 * \brief This prints out the member of the buffer data structure in the given zone.
 * \param [in] zone The debugging zone to place the printout in.
 * \param [in] buf The pointer to the buffer to print.
 * \ingroup group_buffer
 */
void DVP_PrintBuffer(DVP_U32 zone, DVP_Buffer_t *buf);

/*! \brief The data range of the capacity limit's
 * \ingroup group_capacity
 */
#define DVP_CAPACITY_DATA_RANGE (1000)

/*! \brief This function returns the capacity for the requested core.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in] core  Core identifier to get.
 * \returns A number between 0 and DVP_CAPACITY_DATA_RANGE inclusive.
 * \ingroup group_capacity
 */
DVP_U32 DVP_GetCoreCapacity(DVP_Handle handle, DVP_Core_e core);

/*! \brief This function sets the capacity for the requested core, asssuming it is enabled.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in] core  Core identifier to set.
 * \param [in] limit A number between 0 and DVP_CAPACITY_DATA_RANGE inclusive.
 * \ingroup group_capacity
 */
void DVP_SetCoreCapacity(DVP_Handle handle, DVP_Core_e core, DVP_U32 limit);

/*!
 * \brief This function returns information about each core in the DVP system.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in,out] info A preallocated array of DVP_CORE_MAX elems of DVP_CoreInfo_t.
 * Use \ref DVP_Core_e to index the array.
 * \pre DVP_KernelGraph_Init
 * \ingroup group_system
 */
void DVP_QuerySystem(DVP_Handle handle, const DVP_CoreInfo_t info[DVP_CORE_MAX]);

/*!
 * \brief Allows the user to query the system to see which cores implement a
 * kernel.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in] kernel The enumeration of the kernel to search.
 * \param [in,out] present An array of size DVP_CORE_MAX, which indicates if the kernel
 * is implemented on the core (\ref DVP_Core_e is the index to this array).
 * \pre DVP_KernelGraph_Init
 * \ingroup group_kernels
 */
void DVP_QueryKernel(DVP_Handle handle, DVP_KernelNode_e kernel, DVP_BOOL present[DVP_CORE_MAX]);

/*!
 * \brief Allow the user to acculumate an image shift structure over a node. This
 * can be used to determine how an image filter will move the data within the
 * output image relative to the input data.
 * \param [in] handle The handle to DVP returned from \ref DVP_KernelGraph_Init.
 * \param [in] node The pointer to the node data structure.
 * \param [in] shift The pointer to the image shift structure. Values in the
 * shift will not be modified at all before accumulation.
 * \pre \ref DVP_KernelGraph_Verify
 * \ingroup group_graphs
 */
void DVP_KernelGraph_ImageShiftAccum(DVP_Handle handle, DVP_KernelNode_t *node, dvp_image_shift_t *shift);

#ifdef __cplusplus
}
#endif

#endif

