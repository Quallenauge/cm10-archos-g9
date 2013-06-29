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

/*! \file
 * \brief The DVP Internal Memory API. 
 * \author Erik Rainey <erik.rainey@ti.com>
 * \defgroup group_dvp_mem DVP Internal Memory API.
 */

#ifndef _DVP_MEM_INT_H_
#define _DVP_MEM_INT_H_

#include <dvp/dvp_types.h>
#include <sosal/allocator.h>

/*! \brief Recasting the SOSAL allocator type as a DVP_Mem_t. 
 * \ingroup group_dvp_mem
 */
typedef allocator_t DVP_Mem_t;

/*! \brief Recasting the SOSAL allocator dimensionality type as DVP_Dim_t. 
 * \ingroup group_dvp_mem
 */
typedef allocator_dimensions_t DVP_Dim_t;

/*! \brief This function initializes the DVP memory system.
 * \ingroup group_dvp_mem
 */
DVP_Mem_t *dvp_mem_init();

/*! \brief This function disables the memory system 
 * \param [in] pmem The pointer to the pointer to the DVP Memory system. This will be
 * set to NULL during this call. 
 * \ingroup group_dvp_mem
 */
void dvp_mem_deinit(DVP_Mem_t **pmem);

/*! \brief Frees a multidimensional, multipointer buffer. 
 * \param [in] handle The handle to the DVP context. 
 * \param [in] mtype The memory type required. 
 * \param [in] nptrs The number of pointers in the ptrs array.
 * \param [in] ndims The number of dimensions in the dims descriptor.
 * \param [in] dims The pointer to the dimensionality descriptor.
 * \param [in] ptrs The pointer to the array of pointers to free.
 * \ingroup group_dvp_mem
 */
DVP_BOOL dvp_mem_free(DVP_Handle handle,
                      DVP_MemType_e mtype,
                      DVP_S32 nptrs,
                      DVP_S32 ndims,
                      DVP_Dim_t *dims,
                      DVP_PTR *ptrs);


/*!
 * \brief Allocates a multidimensional, multipointer buffer.
Usage: For a linear buffer:
    dvp_mem_calloc(t, 1, 2, {{elemSize, numElems}}, {ptrs}, {handles}, {strides});

Usage: For a 3 plane image of 320x240 of 1 bpp each (like RGBp):
    dvp_mem_calloc(t, 3, 3, {{1,320,240},
                             {1,320,240},
                             {1,320,240}}, {ptrs}, {handles},  {strides});
    Returns:
    {ptrs}    = {%p, %p, %p}
    {handles} = {%p, %p, %p}
    {strides} = {{1,320,0},{1,320,0},{1,320,0}} if 1D, {{1,STRIDE,0},{1,STRIDE,0},{1,STRIDE,0}} if 2D
Usage: For a 3 plane image of 320x240 of 1 bpp for the first plane, but 1/2 planes following (like YUV420):
    dvp_mem_calloc(t, 3, 3, {{1,320,240},
                             {1,160,120},
                             {1,160,120}}, {NULL, NULL, NULL}, {handles},  {strides});
    Returns:
    {ptrs}    = {%p, %p, %p}
    {handles} = {%p, %p, %p}
    {strides} = {{1,320,0},{1,320,0},{1,320,0}} if 1D, {{1,STRIDE,0},{1,STRIDE,0},{1,STRIDE,0}} if 2D
Usage: For a 2 plane image of 320x240 of 1 bpp then 160x120 of 2 bpp (like NV12)
    dvp_mem_calloc(t, 2, 3, {{1,320,240},
                             {2,160,120}}, {NULL, NULL}, {handles}, {strides});

    Returns:
    {ptrs}    = {%p, %p}
    {handles} = {%p, %p}
    {strides} = {{1,320,0},{2,320,0}} if 1D, {{1,STRIDE,0},{2,(other)STRIDE,0}} if 2D
Generically:
    dvp_mem_calloc(type, planes, numDim, {{bpp, width, height},...}, {ptrs}, {handles},  {strides});
 * \param [in] handle The handle to the DVP context. 
 * \param [in] mtype The memory type requested. 
 * \param [in] nptrs The number of pointers in ptrs array.
 * \param [in] ndims The number of valid dimensions in the dims structure. 
 * \param [in] dims The dimensionality structure array.
 * \param [out] ptrs The array of pointers. 
 * \param [out] strides The array of stride values per pointer.
 * \ingroup group_dvp_mem
 */
DVP_BOOL dvp_mem_calloc(DVP_Handle handle,
                        DVP_MemType_e mtype,
                        DVP_S32 nptrs,
                        DVP_S32 ndims,
                        DVP_Dim_t *dims,
                        DVP_PTR *ptrs,
                        DVP_Dim_t *strides);

/*! \brief This function marks the buffer to be flushed.
 * \param [in] handle The handle to the DVP context. 
 * \param [in] mtype The memory type required.
 * \param [in] nptrs The number of pointers in the ptr array.
 * \param [in] ndims The number of dimensional structures in dims.
 * \param [in] dims The array of dimensionality structures. 
 * \param [in] ptrs The array of pointers. 
 * \ingroup group_dvp_mem
 */
DVP_BOOL dvp_mem_flush(DVP_Handle handle,
                       DVP_MemType_e mtype,
                       DVP_S32 nptrs,
                       DVP_S32 ndims,
                       DVP_Dim_t *dims,
                       DVP_PTR *ptrs);

/*! \brief This function marks the buffer to be invalidated. 
 * \param [in] handle The handle to the DVP context. 
 * \param [in] mtype The memory type required.
 * \param [in] nptrs The number of pointers in the ptr array.
 * \param [in] ndims The number of dimensional structures in dims.
 * \param [in] dims The array of dimensionality structures. 
 * \param [in] ptrs The array of pointers. 
 * \ingroup group_dvp_mem
 */
DVP_BOOL dvp_mem_invalidate(DVP_Handle handle,
                            DVP_MemType_e mtype,
                            DVP_S32 nptrs,
                            DVP_S32 ndims,
                            DVP_Dim_t *dims,
                            DVP_PTR *ptrs);

/*! \brief Generates a handle from an existing buffer to be shared with another process.
 * \param [in] handle The handle to the DVP context. 
 * \param [in] mtype The memory type requested. 
 * \param [in] nptrs The number of pointers in the ptrs array.
 * \param [in] ptrs The array of pointers. 
 * \param [out] fds The array to place the shared descriptors. 
 * \ingroup group_dvp_mem
 */
DVP_BOOL dvp_mem_share(DVP_Handle handle,
                       DVP_MemType_e mtype,
                       DVP_S32 nptrs,
                       DVP_PTR *ptrs,
                       DVP_S32 *fds);

/*! \brief Imports a buffer from another process. 
 * \param [in] handle The handle to the DVP context. 
 * \param [in] mtype The memory type requested. 
 * \param [in] ndims The number of dimensions in 
 * \param [in] dims The array of dimensionality structures. 
 * \param [in] nptrs The number of pointers in ptrs.
 * \param [out] ptrs The array of pointers. 
 * \param [out] strides The array of stride value per pointer. 
 * \param [in] fds The array of shared descriptors. 
 * \param [out] hdls The array of handles to each allocated pointer. 
 * \ingroup group_dvp_mem
 */
DVP_BOOL dvp_mem_import(DVP_Handle handle,
                        DVP_MemType_e mtype,
                        DVP_S32 nptrs,
                        DVP_S32 ndims,
                        DVP_Dim_t *dims,
                        DVP_PTR *ptrs,
                        DVP_Dim_t *strides,
                        DVP_S32 *fds,
                        DVP_VALUE *hdls);

/*! \brief Frees an imported buffer from another process. 
 * \param [in] handle The handle to the DVP context. 
 * \param [in] mtype The memory type requested. 
 * \param [in] ndims The number of dimensionality structures in dims. 
 * \param [in] dims The array of dimensionality structures. 
 * \param [in] nptrs The number of pointers in ptrs.
 * \param [out] ptrs The array of pointers. 
 * \param [in] hdls The array of handles for each pointer. 
 * \ingroup group_dvp_mem
 */
DVP_BOOL dvp_mem_free_import(DVP_Handle handle,
                             DVP_MemType_e mtype,
                             DVP_S32 ndims,
                             DVP_Dim_t *dims,
                             DVP_S32 nptrs,
                             DVP_PTR *ptrs,
                             DVP_VALUE *hdls);

/*! \brief An internal function to return the device descriptor of the memory device.
 * \param [in] handle The handle to the DVP context.
 * \return Returns zero when this feature is unsupported.
 * \ingroup group_dvp_mem
 */
int dvp_mem_get_device(DVP_Handle handle);

/*! \brief Converts a DVP memory type to an allocator memory type.
 * \param [in] memType The DVP memory type. 
 * \ingroup group_dvp_mem
 */
allocator_memory_type_e dvp_mem_type_xlate(DVP_MemType_e memType);

/*! \brief Determines if the memory type is cached.
 * \param [in] memType The DVP memory type.
 * \ingroup group_dvp_mem
 */
bool_e dvp_mem_type_cached(DVP_MemType_e memType);

#endif

