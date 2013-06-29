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

#ifndef _SOSAL_ALLOCATOR_H_
#define _SOSAL_ALLOCATOR_H_

/*!
 * \file allocator.h
 * \brief The Allocator API.
 * \author Erik Rainey
 */

#include <sosal/types.h>

#if !defined(_SOSAL_ALLOCATOR_T_DEFINED)
/*! \brief Used to define a type which will be kept internally hidden.
 * \ingroup group_allocator
 */
typedef uint32_t allocator_t;
#endif

/*!
 * \brief The list of supported allocation types.
 * \ingroup group_allocators
 */
typedef enum _allocator_memory_type_e {
    ALLOCATOR_MEMORY_TYPE_VIRTUAL,          /*!< Plain Virtual Memory */
    ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED,/*!< TARGET_DVP_OMAP4 && (SOSAL_USE_ION || SOSAL_USE_TILER) */
    ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED,  /*!< TARGET_DVP_OMAP4 && (SOSAL_USE_ION || SOSAL_USE_TILER) */
    ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED,/*!< TARGET_DVP_OMAP4 && (SOSAL_USE_ION || SOSAL_USE_TILER) */
    ALLOCATOR_MEMORY_TYPE_TILED_2D_CACHED,  /*!< TARGET_DVP_OMAP4 && (SOSAL_USE_ION || SOSAL_USE_TILER) */
    ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED,  /*!< TARGET_DVP_OMAP4 && SOSAL_USE_GRALLOC */
    ALLOCATOR_MEMORY_TYPE_VIRTUAL_SHARED,   /*!< Shared Virtual Memory */
    ALLOCATOR_MEMORY_TYPE_MAX
} allocator_memory_type_e;

/*! \brief The maximum number of dimensions to a single pointer.
 * \todo Exand this to 4?
 * \ingroup group_allocators
 */
#define SOSAL_ALLOCATOR_MAX_DIMS    (3)

/*!
 * The allocator dimensions can be thought of in several ways, either a strict series of dimensional values,
 * or a set of cartesian values, or an image descriptor.
 * \ingroup group_allocators
 */
typedef struct _allocator_dimensions_t {
    union {
        int32_t dims[SOSAL_ALLOCATOR_MAX_DIMS];
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
            //int32_t p;
        } dim;
        struct {
            int32_t bpp;
            int32_t width;
            int32_t height;
            //int32_t planes;
        } img;
    };
} allocator_dimensions_t;

/*! \brief The maximum number of dimensional pointers.
 * \ingroup group_allocators
 */
#define SOSAL_ALLOCATOR_MAX_PTRS (4)

/*! \brief If a native format of memory is not present, clients may use this structure
 * to track allocated memory. It is preferable to use Native formats when
 * possible to avoid needless meta-data redundancy.
 * \ingroup group_allocators
 */
typedef struct _allocator_memory_t {
    /*! A quick way to indicate whether the structure contains allocated memory */
    bool_e allocated;
    /*! The memory type */
    allocator_memory_type_e memtype;
    /*! The pointers to actual memory */
    ptr_t   ptrs[SOSAL_ALLOCATOR_MAX_PTRS];
    /*! The number of pointers valid in ptrs */
    int32_t nptrs;
    /*! The number of dimensions valid in each dims. Each pointer must have the same dimensionality. */
    int32_t ndims;
    /*! The dimensions of each ptrs */
    allocator_dimensions_t dims[SOSAL_ALLOCATOR_MAX_PTRS];
    /*! The stride information of each pointer */
    allocator_dimensions_t strides[SOSAL_ALLOCATOR_MAX_PTRS];
    /*! The handle to each pointer */
    value_t hdls[SOSAL_ALLOCATOR_MAX_PTRS];
    /*! The shared memory descriptors (if shared) */
    int32_t fds[SOSAL_ALLOCATOR_MAX_PTRS];
} allocator_memory_t;

#if defined(SOSAL_USE_BO)
/*! \brief This the the private handle structure in the BO configuration.
 * \ingroup group_allocators
 */
typedef struct _alloc_omap_bo_t {
    void *bo;               /*!< The pointer to the returned kernel data */
    int32_t dmabuf_fd;      /*!< The file descriptor of the mapped memory */
} alloc_omap_bo_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Initializes the Allocator.
 * \ingroup group_allocators
 */
allocator_t *allocator_init();

/*!
 * Deinitializes the Allocator.
 * \ingroup group_allocators
 */
void allocator_deinit(allocator_t **palloc);

/*!
 * \brief Allocates multiple multi-dimensional pointers in one call.
<pre>
Usage: For a linear buffer:
    allocator_calloc(t, 1, 2, {{elemSize, numElems}}, {ptrs}, {handles}, {strides});

Usage: For a 3 plane image of 320x240 of 1 bpp each (like FOURCC_RGBP):
    allocator_calloc(t, 3, 3, {{1,320,240},
                             {1,320,240},
                             {1,320,240}}, {ptrs}, {handles},  {strides});
    Returns:
    {ptrs}    = {%p, %p, %p}
    {handles} = {%p, %p, %p}
    {strides} = {{1,320,0},{1,320,0},{1,320,0}} if 1D, {{1,STRIDE,0},{1,STRIDE,0},{1,STRIDE,0}} if 2D
Usage: For a 3 plane image of 320x240 of 1 bpp for the first plane, but 1/2 planes following (like YUV420):
    allocator_calloc(t, 3, 3, {{1,320,240},
                             {1,160,120},
                             {1,160,120}}, {NULL, NULL, NULL}, {handles},  {strides});
    Returns:
    {ptrs}    = {%p, %p, %p}
    {handles} = {%p, %p, %p}
    {strides} = {{1,320,0},{1,320,0},{1,320,0}} if 1D, {{1,STRIDE,0},{1,STRIDE,0},{1,STRIDE,0}} if 2D
Usage: For a 2 plane image of 320x240 of 1 bpp then 160x120 of 2 bpp (like NV12)
    allocator_calloc(t, 2, 3, {{1,320,240},
                             {2,160,120}}, {NULL, NULL}, {handles}, {strides});

    Returns:
    {ptrs}    = {%p, %p}
    {handles} = {%p, %p}
    {strides} = {{1,320,0},{2,320,0}} if 1D, {{1,STRIDE,0},{2,(other)STRIDE,0}} if 2D
Generically:
    allocator_calloc(type, planes, numDim, {{bpp, width, height},...}, {ptrs}, {handles},  {strides});

Handles:
    Generally the handles array will hold any internally used value which comes from underlying APIs that this API uses. They must be saved and given back with the free function.
</pre>
 * \param alloc The pointer to the allocator meta-data.
 * \param memtype The type of memory to use.
 * \param nptrs The number of pointers to allocate.
 * \param ndims The number of dimensions each pointer will reference.
 * \param dims The array of dimensional values.
 * \param ptrs The array of pointers to set.
 * \param hdls The array of handles to set.
 * \param strides The array of stride information to set.
 * \ingroup group_allocators
 */
bool_e allocator_calloc(allocator_t *alloc,
                        allocator_memory_type_e memtype,
                        int32_t nptrs,
                        int32_t ndims,
                        allocator_dimensions_t *dims,
                        ptr_t *ptrs,
                        value_t *hdls,
                        allocator_dimensions_t *strides);

/*!
 * \brief Frees memory allocated by the calloc function.
 * \ingroup group_allocators
 */
bool_e allocator_free(allocator_t *alloc,
                      allocator_memory_type_e memtype,
                      int32_t nptrs,
                      ptr_t *ptrs,
                      value_t *hdls);

/*!
 * \brief Returns file descriptors to use in sharing memory.
 * \ingroup group_allocators
 */
bool_e allocator_share(allocator_t *alloc,
                       allocator_memory_type_e memtype,
                       int32_t nptrs,
                       value_t *hdls,
                       int32_t *fds);

/*!
 * \brief Imports pre-shared piece of memory into this process space.
 * \ingroup group_allocators
 */
bool_e allocator_import(allocator_t *alloc,
                        allocator_memory_type_e memtype,
                        int32_t nptrs,
                        int32_t ndims,
                        allocator_dimensions_t *dims,
                        ptr_t *ptrs,
                        value_t *hdls,
                        allocator_dimensions_t *strides,
                        int32_t *fds);

/*!
 * \brief Flushes memory from the CPU side cache, if supported by the supplied type.
 * \param alloc
 * \param mtype
 * \param nptrs
 * \param ndims
 * \param dims
 * \param ptrs
 * \param hdls
 * \ingroup group_allocators
 */
bool_e allocator_flush(allocator_t *alloc,
                       allocator_memory_type_e mtype,
                       int32_t nptrs,
                       int32_t ndims,
                       allocator_dimensions_t *dims,
                       ptr_t *ptrs,
                       value_t *hdls);

/*!
 * \brief Invalidate memory (flushes cache line) from the CPU cache, if supported by the supplied type.
 * \param alloc
 * \param mtype
 * \param nptrs
 * \param ndims
 * \param dims
 * \param ptrs
 * \param hdls
 * \ingroup group_allocators
 */
bool_e allocator_invalidate(allocator_t *alloc,
                            allocator_memory_type_e mtype,
                            int32_t nptrs,
                            int32_t ndims,
                            allocator_dimensions_t *dims,
                            ptr_t *ptrs,
                            value_t *hdls);

/*!
 * \brief Retrieves the underlying implementations device descriptor if supported by the supplied type.
 * \param alloc
 * \param mtype
 * \ingroup group_allocators
 */
int allocator_get_device(allocator_t *alloc, allocator_memory_type_e mtype);

/*!
 * \brief Simple wrapper for allocator_calloc
 * \param alloc
 * \param memory
 * \ingroup group_allocators
 */
bool_e allocator_memory_calloc(allocator_t *alloc, allocator_memory_t *memory);

/*!
 * \brief Simple wrapper for allocator_free
 * \param alloc
 * \param memory
 * \ingroup group_allocators
 */
bool_e allocator_memory_free(allocator_t *alloc, allocator_memory_t *memory);

/*!
 * \brief Simple wrapper for allocator_share
 * \param alloc
 * \param memory
 * \ingroup group_allocators
 */
bool_e allocator_memory_share(allocator_t *alloc, allocator_memory_t *memory);

/*!
 * \brief Simple wrapper for allocator_import
 * \param alloc
 * \param memory
 * \ingroup group_allocators
 */
bool_e allocator_memory_import(allocator_t *alloc, allocator_memory_t *memory);

#ifdef __cplusplus
}
#endif

#endif
