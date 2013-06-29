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

#include <sosal/types.h>
#include <sosal/debug.h>
#include <sosal/list.h>

#if defined(SOSAL_USE_SHARED_T)
#include <sosal/shared.h>
#endif

#if defined(SOSAL_USE_TILER) // SYSLINK 1.0 to 2.0 Platforms (QNX, Windows, Android GB/FROYO)
#include <memmgr.h>
#endif

#if defined(SOSAL_USE_ION) // Android ICS
#include <ion/ion.h>
#endif

#if defined(SOSAL_USE_GRALLOC) // Android ICS and GB
#include <hwc/hal_public.h>
#endif

#if defined(SOSAL_USE_BO) // Linux 3.3.0+
#include <omap/omap_drm.h>
#include <libdrm/omap_drmif.h>
#endif

#ifndef PAGE_SHIFT
#define PAGE_SHIFT (12)
#else
#if PAGE_SHIFT != 12
#error "PAGE SHIFT has been defined in an incompatible way!"
#endif
#endif

#if defined(SOSAL_USE_ION)
typedef struct _ion_fd_hdl_t {
    struct ion_handle *handle;
    int fd;
    size_t size;
} ion_fd_hdl_t;
#endif

#define _SOSAL_ALLOCATOR_T_DEFINED
typedef struct _allocator_t {
#if defined(TARGET_DVP_OMAP4) || defined(TARGET_DVP_OMAP5)
#if defined(SOSAL_USE_ION)
    int32_t device;
    list_t fd_list;
#endif
#if defined(SOSAL_USE_GRALLOC)
    IMG_gralloc_module_public_t *module;
    alloc_device_t              *allocator;
#endif
#if defined(SOSAL_USE_BO)
    int32_t drm_fd;
    struct omap_device *drm_dev;
#endif
#endif
    uint32_t reserved;
} allocator_t;

#include <sosal/allocator.h>

#define SOSAL_ALLOCATOR_ALIGN (128)

#if defined(SOSAL_USE_ION)
static int list_hdl_compare(node_t *a, node_t *b)
{
    ion_fd_hdl_t *ad = (ion_fd_hdl_t *)a->data;
    ion_fd_hdl_t *bd = (ion_fd_hdl_t *)b->data;
    if (ad->handle > bd->handle)
        return 1;
    else if (ad->handle < bd->handle)
        return -1;
    else
        return 0;
}
#endif

#if defined(SOSAL_USE_SHARED_T)
/** Returns a randomly generated unique number */
static unique_t allocator_rand()
{
    uint32_t n = 0;
    unique_t un = 0;
    for (n = 0; n < sizeof(unique_t); n++)
    {
        un |= ((rand()&0xFF) << (n*8));
    }
    return un;
}
#endif

/** This returns the SOSAL_PAGE_SIZE aligned size */
static size_t allocator_page_align_size(size_t size)
{
    size_t rsize = size;
    size_t rem = size & (SOSAL_PAGE_SIZE - 1);
    if (rem > 0)
        rsize = size + (SOSAL_PAGE_SIZE - rem);
    SOSAL_PRINT(SOSAL_ZONE_MEM, "RESIZED "FMT_SIZE_T" to "FMT_SIZE_T"\n", size, rsize);
    return rsize;
}

void allocator_deinit(allocator_t **palloc)
{
    if (palloc)
    {
        allocator_t *alloc = *palloc;
        if (alloc)
        {
#if defined(SOSAL_USE_TILER)
            // nothing
#elif defined(SOSAL_USE_ION)
            if (alloc->device > 0)
                ion_close(alloc->device);
            alloc->device = 0;
#endif
#if defined(SOSAL_USE_GRALLOC)
            if (alloc->allocator) {
                gralloc_close(alloc->allocator);
            }
            alloc->allocator = NULL;
            alloc->module = NULL;
#endif
#if defined(SOSAL_USE_BO)
            if (alloc->drm_dev) {
                omap_device_del(alloc->drm_dev);
                alloc->drm_dev = NULL;
                close(alloc->drm_fd);
            }
#endif
            free(alloc);
         }
         *palloc = NULL;
    }
}


allocator_t *allocator_init()
{
    allocator_t *alloc = (allocator_t *)calloc(1, sizeof(allocator_t));
    int32_t errors = 0;
    if (alloc)
    {
#if defined(SOSAL_USE_TILER)
        alloc->reserved = 1;
#elif defined(SOSAL_USE_ION)
        if (errors == 0)
        {
            alloc->device = ion_open();
            if (alloc->device < 0)
                errors++;
        }
#endif
#if defined(SOSAL_USE_GRALLOC)
        if (errors == 0)
        {
            int32_t ret = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const struct hw_module_t **)&alloc->module);
            SOSAL_PRINT(SOSAL_ZONE_MEM, "hw_get_module(%s,...) returned %d\n", GRALLOC_HARDWARE_MODULE_ID, ret);
            if (ret < 0)
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to open Gralloc HW\n");
                errors++;
            }
            SOSAL_PRINT(SOSAL_ZONE_MEM, "hw_module_t *%p by %s\n", alloc->module, alloc->module->base.common.author);
            ret = gralloc_open((const struct hw_module_t *)alloc->module, &alloc->allocator);
            if (ret < 0)
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to open Gralloc Allocator!\n");
                errors++;
            }
        }
#endif
#if defined(SOSAL_USE_BO)
        if (errors == 0)
        {
            alloc->drm_fd = drmOpen("omapdrm", NULL);
            if (alloc->drm_fd < 0)
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to open OMAPDRM device!\n");
                errors++;
            }
            else
            {
                alloc->drm_dev = omap_device_new(alloc->drm_fd);
                if (alloc->drm_dev == NULL)
                {
                    SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to create new OMAP device structure!\n");
                    errors++;
                }
            }
        }
#endif
        if (errors > 0)
        {
            allocator_deinit(&alloc);
        }
    }
    return alloc;
}

int allocator_get_device(allocator_t *alloc, allocator_memory_type_e mtype)
{
    int fd = 0;

    if (alloc == NULL ||
        mtype == ALLOCATOR_MEMORY_TYPE_VIRTUAL)
        return 0;

#if defined(SOSAL_USE_ION)
    if (mtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED ||
        mtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED   ||
        mtype == ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED ||
        mtype == ALLOCATOR_MEMORY_TYPE_TILED_2D_CACHED)
        fd = alloc->device;
#endif
    return fd;
}

bool_e allocator_flush(allocator_t *alloc,
                       allocator_memory_type_e mtype,
                       int32_t nptrs,
                       int32_t ndims,
                       allocator_dimensions_t *dims,
                       ptr_t *ptrs,
                       value_t *hdls)
{
    bool_e ret = true_e; // lie to the upper layers if it's not the right type

    if (alloc == NULL || mtype > ALLOCATOR_MEMORY_TYPE_MAX || nptrs <= 0 || ndims <= 0 || dims == NULL || ptrs == NULL)
        return false_e;

#if defined(SOSAL_USE_ION)
    if (nptrs > 0 && mtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED && hdls != NULL)
    {
        int32_t n,p;
        uint32_t size;
        ret = true_e;
        for (p = 0; p < nptrs; p++)
        {
            if (hdls[p] == 0)
                continue;

            // calculate the size of the plane
            size = 1;
            for (n = 0; n < ndims; n++)
                size *= dims[p].dims[n];
#if defined(SOSAL_USE_ION_CACHING)
            SOSAL_PRINT(SOSAL_ZONE_MEM, "Flushing PTR:%p HDL:"FMT_VALUE_T" SIZE:%u\n", ptrs[p], hdls[p], size);
            if (ion_flush_cached(alloc->device, (struct ion_handle *)hdls[p], size, ptrs[p]) < 0)
                ret = false_e;
#endif
        }
    }
    else
    {
        SOSAL_PRINT(SOSAL_ZONE_WARNING, "Memory type %d does not need to be flushed!\n", mtype);
    }
#endif
    return ret;
}

bool_e allocator_invalidate(allocator_t *alloc,
                            allocator_memory_type_e mtype,
                            int32_t nptrs,
                            int32_t ndims,
                            allocator_dimensions_t *dims,
                            ptr_t *ptrs,
                            value_t *hdls)
{
    bool_e ret = true_e; // lie to the upper layers if it's not the right type

    if (alloc == NULL || mtype > ALLOCATOR_MEMORY_TYPE_MAX || nptrs <= 0 || ndims <= 0 || dims == NULL || ptrs == NULL)
        return false_e;

#if defined(SOSAL_USE_ION)
    if (nptrs > 0 && mtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED)
    {
        int32_t n,p;
        uint32_t size;
        ret = true_e;
        for (p = 0; p < nptrs; p++)
        {
            if (hdls[p] == 0)
                continue;

            // calculate the size of the plane
            size = 1;
            for (n = 0; n < ndims; n++)
                size *= dims[p].dims[n];

#if defined(SOSAL_USE_ION_CACHING)
            SOSAL_PRINT(SOSAL_ZONE_MEM, "Invalidating PTR:%p HDL:"FMT_VALUE_T" SIZE:%u\n", ptrs[p], hdls[p], size);
            if (ion_inval_cached(alloc->device, (struct ion_handle *)hdls[p], size, ptrs[p]) < 0)
                ret = false_e;
#endif
        }
    }
    else {
        SOSAL_PRINT(SOSAL_ZONE_WARNING, "Memory type %d does not need to be invalidated!\n", mtype);
    }
#endif
    return ret;
}

bool_e allocator_free(allocator_t *alloc,
                      allocator_memory_type_e mtype,
                      int32_t nptrs,
                      ptr_t *ptrs,
                      value_t *hdls)
{
    int32_t err = 0;
    bool_e ret = true_e;
    int32_t p = 0;

    if (alloc == NULL || nptrs <= 0 || ptrs == NULL)
        return false_e;

    switch (mtype)
    {
#if defined(SOSAL_USE_TILER)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED:
            // hdls can be NULL, but shouldn't be.

            for (p = 0; p < nptrs; p++) {
                if (ptrs[p] == NULL)
                    continue;
                SOSAL_PRINT(SOSAL_ZONE_MEM, "TILER: Freeing %p\n", ptrs[p]);
                err = MemMgr_Free(ptrs[p]);
                if (err != MEMMGR_ERR_NONE)
                {
                    SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: Failed to free TILER buffer %p (err=%d)\n", __FUNCTION__, ptrs[p], err);
                }
                ptrs[p] = NULL;
            }
            break;
#elif defined(SOSAL_USE_ION)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED:
            if (hdls == NULL)
                return false_e;
            for (p = 0; p < nptrs; p++) {
                node_t node, *tmp;
                ion_fd_hdl_t fd_hdl = {
                    .handle = 0,
                    .fd = 0,
                };
                if (ptrs[p] == NULL || hdls[p] == 0)
                    continue;
                SOSAL_PRINT(SOSAL_ZONE_MEM, "ION: Freeing %p Hdl:"FMT_VALUE_T"\n", ptrs[p],hdls[p]);
                if (ion_free(alloc->device, (struct ion_handle *)hdls[p]) < 0) {
                    SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: Failed to free ION hdl: "FMT_VALUE_T" ptr: %p\n", __FUNCTION__, hdls[p], ptrs[p]);
                }
                fd_hdl.handle = (struct ion_handle *)hdls[p];
                node.data = (value_t)&fd_hdl;
                tmp = list_remove_match(&alloc->fd_list, &node, list_hdl_compare);
                if (tmp)
                {
                    ion_fd_hdl_t *fd_hdl = (ion_fd_hdl_t *)tmp->data;
                    if (munmap(ptrs[p], fd_hdl->size) < 0) {
                        SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: Failed to munmap ION hdl: "FMT_VALUE_T" ptr: %p\n", __FUNCTION__, hdls[p], ptrs[p]);
                    }
                    close(fd_hdl->fd);
                    free(fd_hdl);
                    free(tmp);
                }
                hdls[p] = 0;
                ptrs[p] = NULL;
            }
            break;
#endif
#if defined(SOSAL_USE_GRALLOC)
        case ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED:
            if (hdls == NULL)
                return false_e;
            // no unmapping needed for GRALLOC
            for (p = 0; p < nptrs; p++) {
                if (hdls[p] != 0) { // only try to free valid handles.
                    SOSAL_PRINT(SOSAL_ZONE_MEM, "GRALLOC: Freeing %p Hdl:"FMT_VALUE_T"\n", ptrs[p], hdls[p]);
                    err = alloc->module->base.unlock((const struct gralloc_module_t *)alloc->module, (buffer_handle_t)hdls[p]);
                    ptrs[p] = NULL;
                    err = alloc->allocator->free((struct alloc_device_t *)alloc->allocator, (buffer_handle_t)hdls[p]);
                    if (err < 0) {
                        SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: Failed to free GRALLOC buffer hdl "FMT_VALUE_T"\n", __FUNCTION__, hdls[p]);
                        ret = false_e;
                    }
                }
                hdls[p] = 0;
            }
            break;
#endif
#if defined(SOSAL_USE_BO)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED:
            if (hdls == NULL)
                return false_e;
            for (p = 0; p < nptrs; p++) {
                if (hdls[p]) {
                    alloc_omap_bo_t *bot = (alloc_omap_bo_t *)hdls[p];
                    SOSAL_PRINT(SOSAL_ZONE_MEM, "BO: Freeing %p\n", bot->bo);
                    close(bot->dmabuf_fd);
                    omap_bo_del((struct omap_bo *)bot->bo); // this unmaps for us
                    free(bot);
                    hdls[p] = 0;
                    ptrs[p] = NULL;
                }
            }
            break;
#endif
#if defined(SOSAL_USE_TILER)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED:
#endif
        case ALLOCATOR_MEMORY_TYPE_VIRTUAL:
            for (p = 0; p < nptrs; p++) {
                if (ptrs[p] == NULL)
                    continue;
                SOSAL_PRINT(SOSAL_ZONE_MEM, "Freeing %p\n", ptrs[p]);
                free(ptrs[p]);
                ptrs[p] = NULL;
            }
            break;
#if defined(SOSAL_USE_SHARED_T)
        case ALLOCATOR_MEMORY_TYPE_VIRTUAL_SHARED:
        {
            for (p = 0; p < nptrs; p++)
            {
#if defined(SOSAL_DEBUG)
                shared_t *shm = (shared_t *)hdls[p];
                SOSAL_PRINT(SOSAL_ZONE_MEM, "SHM: Freeing %p, UN:0x%08x\n", ptrs[p], shm->un);
#endif
                shared_free((shared_t **)&hdls[p]);
                ptrs[p] = NULL;
            }
            break;
        }
#endif
        default:
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: Unknown memory type %d\n", __FUNCTION__, mtype);
            ret = false_e;
            break;
    }
    return ret;
}


bool_e allocator_calloc(allocator_t *alloc,
                        allocator_memory_type_e mtype,
                        int32_t nptrs,
                        int32_t ndims,
                        allocator_dimensions_t *dims,
                        ptr_t *ptrs,
                        value_t *hdls,
                        allocator_dimensions_t *strides)
{
    bool_e ret = true_e;
    uint32_t size = 0;
    int32_t p = 0, n = 0;
    uint32_t flags = 0;

    if (alloc == NULL || nptrs <= 0 || ndims <= 0 || dims == NULL || ptrs == NULL)
        return false_e;

    SOSAL_PRINT(SOSAL_ZONE_MEM, "%s: Attempting allocation of %d ptrs of type %d\n", __FUNCTION__, nptrs, mtype);

    // Remove warnings about unused variables
    flags |= 0;

    switch (mtype)
    {
#if defined(SOSAL_USE_TILER)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED:
        {
            MemAllocBlock *blocks = calloc(nptrs, sizeof(MemAllocBlock));
            if (blocks)
            {
                for (p = 0; p < nptrs; p++)
                {
                    size = 1;
                    for (n = 0; n < ndims; n++)
                        size *= dims[p].dims[n];

                    size = allocator_page_align_size(size);
                    if (mtype == ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED)
                    {
                        blocks[p].dim.area.width = dims[p].img.width;
                        blocks[p].dim.area.height = dims[p].img.height;
                        switch (dims[p].img.bpp)
                        {
                            case sizeof(uint8_t):
                                blocks[p].pixelFormat = PIXEL_FMT_8BIT;
                                break;
                            case sizeof(uint16_t):
                                blocks[p].pixelFormat = PIXEL_FMT_16BIT;
                                break;
                            case 3: // triples
                                blocks[p].pixelFormat = PIXEL_FMT_8BIT;
                                blocks[p].dim.area.width *= 3;
                                break;
                            case sizeof(uint32_t):
                                blocks[p].pixelFormat = PIXEL_FMT_32BIT;
                                break;
                            default:
                                break;
                        }
                    }
                    else
                    {
                        blocks[p].dim.len = size;
                        blocks[p].pixelFormat = PIXEL_FMT_PAGE;
                    }
                    SOSAL_PRINT(SOSAL_ZONE_MEM, "%s: Allocating TYPE:%d FMT:%d DIM:%ux%u\n",
                                __FUNCTION__,
                                mtype,
                                blocks[p].pixelFormat,
                                blocks[p].dim.area.width,
                                blocks[p].dim.area.height);

                }
                hdls[0] = (value_t)MemMgr_Alloc(blocks, nptrs);
                if (hdls[0])
                {
                    for (p = 0; p < nptrs; p++)
                    {
                        strides[p].dim.x = dims[p].img.bpp;
                        if (mtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED)
                        {
                            strides[p].dim.y = strides[p].dim.x*dims[p].img.width;
                            strides[p].dim.z = strides[p].dim.y*dims[p].img.height;
                        }
                        else
                        {
                            strides[p].dim.y = blocks[p].stride;
                            strides[p].dim.z = blocks[p].stride*dims[p].img.height;
                        }
                        if (p == 0)
                            ptrs[p] = blocks[p].ptr;
                        else
                            ptrs[p] = &((uint8_t *)ptrs[p-1])[strides[p].dim.z];
                    }
                }
                else
                {
                    SOSAL_PRINT(SOSAL_ZONE_MEM, "%s: Failed to allocate from TILER!\n", __FUNCTION__);
                    ret = false_e;
                }
                free(blocks);
            }
            break;
        }
#elif defined(SOSAL_USE_ION)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED:
            flags = (1<< OMAP_ION_HEAP_SECURE_INPUT); // overload the secure input space
            // fall-through
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED:
        {
            int32_t status = 0;
            size_t rsize = 0;
            for (p = 0; p < nptrs; p++)
            {
                int fmt = 0;
                size_t stride = 0;
                switch (dims[p].img.bpp)
                {
                    case 1:
                        fmt = TILER_PIXEL_FMT_8BIT;
                        break;
                    case 2:
                        fmt = TILER_PIXEL_FMT_16BIT;
                        break;
                    case 3:
                    case 4:
                        fmt = TILER_PIXEL_FMT_32BIT;
                        break;
                    default:
                        fmt = TILER_PIXEL_FMT_PAGE;
                        break;
                }
                size = 1; // reset to a good value
                for (n = 0; n < ndims; n++) {
                    size *= dims[p].dims[n];
                    strides[p].dims[n] = size; // initialize to this, but use value from alloc later if 2D
                }
                rsize = allocator_page_align_size(size);
                SOSAL_PRINT(SOSAL_ZONE_MEM, "%s: Allocating ION Buffer FMT:%d DIM:%ux%u FLAGS=%08x SIZE=%zu RSIZE=%zu (div=%lu rem=%lu)\n",
                            __FUNCTION__,
                            fmt,
                            dims[p].img.width,
                            dims[p].img.height,
                            flags,
                            size, rsize,
                            rsize/SOSAL_PAGE_SIZE, rsize%SOSAL_PAGE_SIZE);
                if (mtype == ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED) {
#if defined(SOSAL_USE_ION_TILER)
                    status = ion_alloc_tiler(alloc->device, dims[p].img.width, dims[p].img.height, fmt, flags, (struct ion_handle **)&hdls[p], &stride);
#else
                    // when we don't have 2D memory from TILER, simulate it.
                    stride = dims[p].img.width * dims[p].img.bpp;
                    status = ion_alloc(alloc->device, rsize, SOSAL_ALLOCATOR_ALIGN, flags, (struct ion_handle **)&hdls[p]);
#endif
                    strides[p].dim.y = (int32_t)stride;
                    strides[p].dim.z = strides[p].dim.y*dims[p].img.height;
                } else
                    status = ion_alloc(alloc->device, rsize, SOSAL_ALLOCATOR_ALIGN, flags, (struct ion_handle **)&hdls[p]);
                if (status < 0 || hdls[p] == 0) {
                    SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: ION Allocation failed, status=%d, errno=%d hdl:"FMT_VALUE_T"\n", __FUNCTION__, status, errno, hdls[p]);
                    ret = false_e;
                } else {
                    ion_fd_hdl_t *fd_hdl = calloc(1, sizeof(ion_fd_hdl_t));
                    if (fd_hdl)
                    {
                        SOSAL_PRINT(SOSAL_ZONE_MEM, "%s: Allocated ION Buffer HDL:"FMT_VALUE_T"\n", __FUNCTION__, hdls[p]);
#if defined(SOSAL_USE_ION_CACHING)
                        if (mtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED) // 1D cached
                            status = ion_map_cacheable(alloc->device, (struct ion_handle*)hdls[p], rsize, PROT_READ|PROT_WRITE, MAP_SHARED, 0, (uint8_t **)&ptrs[p], &fd_hdl->fd);
                        else // 1D, 2D uncached
#endif
                        {
                            if (mtype == ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED)
                                rsize = dims[p].img.height * strides[p].dim.y;
                            status = ion_map(alloc->device, (struct ion_handle *)hdls[p], rsize, PROT_READ|PROT_WRITE, MAP_SHARED, 0, (uint8_t **)&ptrs[p], &fd_hdl->fd);
                        }
                        if (status < 0) {
                            SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: ION Mapping Failed, status=%d, errno=%d\n", __FUNCTION__, status, errno);
                            ret = false_e;
                        } else {
                            node_t *node = node_create((value_t)fd_hdl);
                            if (node)
                            {
                                fd_hdl->handle = (struct ion_handle *)hdls[p];
                                fd_hdl->size = rsize;
                                list_append(&alloc->fd_list, node);
                            }
                            else
                            {
                                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to remember fd to hdl relation. LEAK!\n");
                            }
                            /** \note Memset'ing the ION buffer can be a VERY SLOW OPERATION! */
                            if (mtype == ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED) {
                                int32_t y;
                                uint8_t *tmp = (uint8_t *)ptrs[p];
                                for (y = 0; y < dims[p].img.height; y++)
                                    memset(&tmp[y * strides[p].dim.y], 0, strides[p].dim.x*dims[p].img.width);
                            } else
                                memset(ptrs[p], 0, rsize);
                        }
                    }
                    else
                    {
                        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR: Failed to create association structure!\n");
                        ret = false_e;
                    }
                }
            }
            break;
        }
#endif
#if defined(SOSAL_USE_GRALLOC)
        case ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED:
        {
            int32_t status = 0;
            if (hdls == NULL || alloc->module == NULL || alloc->allocator == NULL)
                return false_e;

            if (nptrs != 2 || ndims != 3 || dims[0].dim.x != 1 || dims[1].dim.x != 2) // must be allocating NV12 only
                return false_e;

            int width = dims[0].img.width;
            int height = dims[0].img.height;
            int format = 0x100;
            int usage = GRALLOC_USAGE_HW_RENDER |
                        GRALLOC_USAGE_HW_TEXTURE |
                        GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_SW_WRITE_NEVER;

            memset(strides, 0, nptrs*sizeof(allocator_dimensions_t));
            strides[0].dim.x = dims[0].dim.x;
            strides[1].dim.x = dims[0].dim.x;

            SOSAL_PRINT(SOSAL_ZONE_MEM, "%s: Allocating GRALLOC FMT:%x DIM:%ux%u USAGE:%x\n", __FUNCTION__, format, width, height, usage);

            status = alloc->allocator->alloc((struct alloc_device_t *)alloc->allocator, width, height, format, usage, (buffer_handle_t *)&hdls[0], (int *)&strides[0].dim.y);
            if (status == 0)
            {
                // remember the stride for both planes
                strides[1].dim.y = strides[0].dim.y;

                // calculate the plane size and store in dim.z
                strides[0].dim.z = strides[0].dim.y*dims[0].img.height;
                strides[1].dim.z = strides[1].dim.y*dims[1].img.height;

                // fill in the pointers
                status = alloc->module->base.lock((const struct gralloc_module_t *)alloc->module, (buffer_handle_t)hdls[0], usage, 0, 0, width, height, ptrs);
                if (status != 0)
                {
                    SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: Failed to lock GRALLOC buffer (status=%d errno=%d)\n", __FUNCTION__, status, errno);
                    ret = false_e;
                }
            }
            else
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: Failed to alloc GRALLOC buffer (status=%d errno=%d)\n", __FUNCTION__, status, errno);
                ret = false_e;
            }
            break;
        }
#endif
#if defined(SOSAL_USE_BO)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED:
        {
            flags |= OMAP_BO_SCANOUT; // all memory must be phys contig for SIMCOP to be able to access
            if (mtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED)
                flags |= (OMAP_BO_CACHED);
            else
                flags |= (OMAP_BO_WC); // make uncached memory have a better write speed.
            for (p = 0; p < nptrs; p++)
            {
                size_t size = 1;
                alloc_omap_bo_t *bot = (alloc_omap_bo_t *)calloc(1, sizeof(alloc_omap_bo_t));
                if (bot == NULL)
                    break;

                if (mtype == ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED ||
                    mtype == ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED)
                {
                    strides[p].dims[0] = dims[p].img.bpp;
                    switch (dims[p].img.bpp)
                    {
                        case 1:
                            flags |= OMAP_BO_TILED_8;
                            break;
                        case 2:
                            flags |= OMAP_BO_TILED_16;
                            break;
                        case 3: // put 24 bit values in 32 bit plane for easy rotation
                            strides[p].dims[0] = 4;
                            // fallthrough
                        case 4:
                            flags |= OMAP_BO_TILED_32;
                            break;
                        default:
                            break;
                    }
                    bot->bo = omap_bo_new_tiled(alloc->drm_dev, dims[p].img.width, dims[p].img.height, flags);
                }
                else
                {
                    for (n = 0; n < ndims; n++)
                        size *= dims[p].dims[n];
                    strides[p].dims[0] = dims[p].img.bpp;
                    bot->bo = omap_bo_new(alloc->drm_dev, size, flags);
                }
                if (bot->bo)
                {
                    ptrs[p] = omap_bo_map((struct omap_bo *)bot->bo);
                    if (ptrs[p])
                    {
                        strides[p].dims[1] = dims[p].img.width * dims[p].img.bpp;
                        if (mtype == ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED ||
                            mtype == ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED)
                        {
                            #define ALIGN2(x,n) (((x) + ((1 << (n)) - 1)) & ~((1 << (n)) - 1))
                            strides[p].dims[1] = ALIGN2(strides[p].dims[1],PAGE_SHIFT);
                        }
                        strides[p].dims[2] = strides[p].dims[1] * dims[p].img.height;
                        hdls[p] = (value_t)bot; // the returned handle is an (alloc_omap_bo_t *)
                        bot->dmabuf_fd = omap_bo_dmabuf((struct omap_bo *)bot->bo);
                        SOSAL_PRINT(SOSAL_ZONE_MEM, "%s: Allocated BO %p MAP %p STRIDE:%u FD:%d\n", __FUNCTION__, bot->bo, ptrs[p], strides[p].dims[1], bot->dmabuf_fd);
                        ret = true_e;
                    }
                    else
                    {
                        SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to map OMAP buffer object %p to our space!\n", bot->bo);
                        omap_bo_del((struct omap_bo *)bot->bo);
                        ret = false_e;
                        break;
                    }
                }
                else
                {
                    SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to allocate buffer object!\n");
                    ret = false_e;
                    break;
                }
            }
            break;
        }
#endif
#if defined(SOSAL_USE_TILER)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED:
#endif
        case ALLOCATOR_MEMORY_TYPE_VIRTUAL:
        {
            uint32_t rsize = 0;
            for (p = 0; p < nptrs; p++) {
                size = 1; // reset to good value
                for (n = 0; n < ndims; n++) {
                    size *= dims[p].dims[n];
                    strides[p].dims[n] = size;
                }
                // if we're running on an OMAP, the SIMCOP needs page aligned data.
                rsize = allocator_page_align_size(size);
#if defined(ANDROID)
                ptrs[p] = memalign(SOSAL_PAGE_SIZE, rsize);
                if (ptrs[p]) memset(ptrs[p], 0, rsize);
#elif defined(__QNX__) || defined(LINUX)
                int32_t err = posix_memalign(&ptrs[p], SOSAL_PAGE_SIZE, rsize);
                if (err == ENOMEM) {
                    ret = false_e;
                    ptrs[p] = NULL;
                    return ret;
                } else if (err == EINVAL) {
                    SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: memalign failed due to alignment request!\n", __FUNCTION__);
                    ret = false_e;
                    ptrs[p] = NULL;
                    return ret;
                }
                if (ptrs[p])
                    memset(ptrs[p], 0, size);
#else
                ptrs[p] = (ptr_t)calloc(1, size);
#endif
                if (ptrs[p] == NULL) {
                    ret = false_e;
                    for (p = p-1; p > -1; p--) {
                        free(ptrs[p]);
                        ptrs[p] = NULL;
                    }
                    break; // exit
                }
            }
            break;
        }
#if defined(SOSAL_USE_SHARED_T)
        case ALLOCATOR_MEMORY_TYPE_VIRTUAL_SHARED:
        {
            for (p = 0; p < nptrs; p++)
            {
                shared_t *shm = NULL;
                unique_t un = allocator_rand();
                size = 1;
                for (n = 0; n < ndims; n++)
                {
                    size *= dims[p].dims[n];
                    strides[p].dims[n] = size;
                }
                shm = shared_alloc(un, size);
                if (shm)
                {
                    SOSAL_PRINT(SOSAL_ZONE_MEM, "SHM: UN:0x%08x SIZE:"FMT_SIZE_T"\n", shm->un, shm->size);
                    hdls[p] = (value_t)shm;
                    ptrs[p] = shm->data;
                }
                else
                {
                    ret = false_e;
                    break;
                }
            }
            break;
        }
#endif
        default:
            ret = false_e;
            break;
    }
#if defined(SOSAL_DEBUG)
    if (ret == true_e) {
        for (p = 0; p < nptrs; p++) {
            SOSAL_PRINT(SOSAL_ZONE_MEM, "%s: Callocated type %d [%u] ptr:%p hdl:"FMT_VALUE_T" dim:{%d,%d,%d} strides:{%d,%d,%d}\n", __FUNCTION__, mtype, p, ptrs[p], hdls[p], dims[p].dim.x,dims[p].dim.y,dims[p].dim.z,strides[p].dim.x,strides[p].dim.y,strides[p].dim.z);
        }
    }
#endif
    if (ret == false_e)
    {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s: Failed to allocate memory type %u!\n", __FUNCTION__, mtype);
    }
    return ret;
}

bool_e allocator_share(allocator_t *alloc,
                       allocator_memory_type_e memtype,
                       int32_t nptrs,
                       value_t *hdls,
                       int32_t *fds)
{
    int32_t p = 0;
    bool_e ret = false_e;
    if (alloc == NULL || memtype > ALLOCATOR_MEMORY_TYPE_MAX || nptrs <= 0 || hdls == NULL || fds == NULL)
        return false_e;

    switch (memtype)
    {
#if defined(SOSAL_USE_ION)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED:
        {
            int err = 0;
            for (p = 0; p < nptrs; p++)
            {
                err = ion_share(alloc->device, (struct ion_handle *)hdls[p], &fds[p]);
                SOSAL_PRINT(SOSAL_ZONE_MEM, "ION_SHARE returned %d, errno=%d, share_fd=%d\n", err, errno, fds[p]);
            }
            ret = (err == 0) ? true_e : false_e;
            break;
        }
#endif
#if defined(DVP_USE_BO)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED:
            for (p = 0; p < nptrs; p++)
            {
                alloc_omap_bo_t *bot = (alloc_omap_bo_t *)hdls[p];
                fds[p] = dup(bot->dmabuf_fd); // pass a duplicate to remote processes.
            }
            ret = true_e;
            break;
#endif
#if defined(SOSAL_USE_SHARED_T)
        case ALLOCATOR_MEMORY_TYPE_VIRTUAL_SHARED:
            for (p = 0; p < nptrs; p++)
            {
                shared_t *shm = (shared_t *)hdls[p];
                fds[p] = (int32_t)shm->un;
            }
            ret = true_e;
            break;
#endif
        default:
            break;
    }
    return ret;
}

bool_e allocator_import(allocator_t *alloc,
                        allocator_memory_type_e memtype,
                        int32_t nptrs,
                        int32_t ndims,
                        allocator_dimensions_t *dims,
                        ptr_t *ptrs,
                        value_t *hdls,
                        allocator_dimensions_t *strides,
                        int32_t *fds)
{
    int32_t n = 0, p = 0;
    uint32_t size = 0;
    bool_e ret = false_e;
    if (alloc == NULL || memtype > ALLOCATOR_MEMORY_TYPE_MAX || nptrs <= 0 || ndims <= 0 || dims == NULL || ptrs == NULL || hdls == NULL || fds == NULL)
        return false_e;

    switch (memtype)
    {
#if defined(SOSAL_USE_ION)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED:
        {
            int err = 0;
            for (p = 0; p < nptrs; p++)
            {
                size_t rsize = 0;
                ion_fd_hdl_t *fd_hdl = calloc(1, sizeof(ion_fd_hdl_t));
                if (fd_hdl)
                {
                    err = ion_import(alloc->device, fds[p], (struct ion_handle **)&hdls[p]);
                    SOSAL_PRINT(SOSAL_ZONE_MEM, "ION_IMPORT returned %d, errno=%d, share_fd=%d\n", ret, errno, fds[p]);

                    size = 1; // reset to a good value
                    for (n = 0; n < ndims; n++) {
                        size *= dims[p].dims[n];
                        strides[p].dims[n] = size; // initialize to this, but use value from alloc later if 2D
                    }
                    rsize = allocator_page_align_size(size);
#if defined(SOSAL_USE_ION_CACHING)
                    if (memtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED)
                        err = ion_map_cacheable(alloc->device, (struct ion_handle *)hdls[p], rsize, PROT_READ|PROT_WRITE, MAP_SHARED, 0, (uint8_t **)&ptrs[p], &fd_hdl->fd);
                    else
#endif
                        err = ion_map(alloc->device, (struct ion_handle *)hdls[p], rsize, PROT_READ|PROT_WRITE, MAP_SHARED, 0, (uint8_t **)&ptrs[p], &fd_hdl->fd);
                    SOSAL_PRINT(SOSAL_ZONE_MEM, "ION MAP returned %d errno=%d\n", ret, errno);
                    if (err < 0)
                    {
                        SOSAL_PRINT(SOSAL_ZONE_MEM, "ION SHARING Failed! (err=%d errno=%d)\n", err, errno);
                        ret = false_e;
                        break;
                    }
                    else
                    {
                        fd_hdl->handle = (struct ion_handle *)hdls[p];
                        fd_hdl->size = rsize;
                        node_t *node = node_create((value_t)fd_hdl);
                        if (node)
                            list_append(&alloc->fd_list, node);
                        else
                        {
                            SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR: Failed to remember file descriptor. LEAK!\n");
                            ret = false_e;
                            break;
                        }
                    }
                }
                else
                {
                    SOSAL_PRINT(SOSAL_ZONE_ERROR, "ION Failed to allocate assocation structure.\n");
                    ret = false_e;
                    break;
                }
            }
            if (err == 0)
                ret = true_e;
            break;
        }
#endif
#if defined(DVP_USE_BO)
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_CACHED:
        case ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED:
        case ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED:
            for (p = 0; p < nptrs; p++)
            {
                // not supported...
            }
            break;
#endif
#if defined(SOSAL_USE_SHARED_T)
        case ALLOCATOR_MEMORY_TYPE_VIRTUAL_SHARED:
            for (p = 0; p < nptrs; p++)
            {
                shared_t * shm = NULL;
                size = 1;
                for (n = 0; n < ndims; n++)
                {
                    size *= dims[p].dims[n];
                    strides[p].dims[n] = size;
                }
                shm = shared_alloc((unique_t)fds[p], size);
                if (shm)
                {
                    SOSAL_PRINT(SOSAL_ZONE_MEM, "SHM: UN:0x%08x SIZE:"FMT_SIZE_T"\n", shm->un, shm->size);
                    hdls[p] = (value_t)shm;
                    ptrs[p] = shm->data;
                }
                else
                {
                    ret = false_e;
                    break;
                }
            }
            ret = true_e;
            break;
#endif
        default:
            break;
    }
    return ret;
}

bool_e allocator_memory_calloc(allocator_t *alloc, allocator_memory_t *memory)
{
    if (alloc && memory && memory->allocated == false_e)
    {
        bool_e ret = allocator_calloc(alloc,
                                      memory->memtype,
                                      memory->nptrs,
                                      memory->ndims,
                                      memory->dims,
                                      memory->ptrs,
                                      memory->hdls,
                                      memory->strides);
        memory->allocated = ret;
        return ret;
    }
    return false_e;
}

bool_e allocator_memory_free(allocator_t *alloc, allocator_memory_t *memory)
{
    if (alloc && memory && memory->allocated == true_e)
    {
        if (allocator_free(alloc,
                           memory->memtype,
                           memory->nptrs,
                           memory->ptrs,
                           memory->hdls) == true_e)
        {
            int32_t f = 0;
            memory->allocated = false_e;
#if defined(SOSAL_USE_ION) || defined(SOSAL_USE_BO) || defined(SOSAL_USE_TILER)
            for (f = 0; f < memory->nptrs; f++) {
                if (memory->memtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED ||
                    memory->memtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED)
                {
                    close(memory->fds[f]);
                }
            }
#endif
            return true_e;
        }
    }
    return false_e;
}

bool_e allocator_memory_share(allocator_t *alloc, allocator_memory_t *memory)
{
    if (alloc && memory && memory->allocated)
    {
        return allocator_share(alloc,
                               memory->memtype,
                               memory->nptrs,
                               memory->hdls,
                               memory->fds);
    }
    return false_e;
}

bool_e allocator_memory_import(allocator_t *alloc, allocator_memory_t *memory)
{
    if (alloc && memory && memory->allocated == false_e)
    {
        memory->allocated = allocator_import(alloc,
                                             memory->memtype,
                                             memory->nptrs,
                                             memory->ndims,
                                             memory->dims,
                                             memory->ptrs,
                                             memory->hdls,
                                             memory->strides,
                                             memory->fds);
        return memory->allocated;
    }
    return false_e;
}

#include <sosal/image.h>
#include <sosal/fph.h>

bool_e allocator_unittest(int argc, char *argv[])
{
    uint32_t width = 320;
    uint32_t height = 240;
    uint32_t numImages = 4;
    int32_t errors = 0;
    fph_t *handle_hash = fph_init(64,64,NULL);
    if (argc > 3)
    {
        width = atoi(argv[1]);
        height = atoi(argv[2]);
        numImages = atoi(argv[3]);
    }

    if (numImages > 0 && width > 0 && height > 0)
    {
        uint32_t i, t, p;
        allocator_memory_type_e types[] = {
            ALLOCATOR_MEMORY_TYPE_VIRTUAL,
            ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED,
            ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED,
            ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED,
            ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED,
            ALLOCATOR_MEMORY_TYPE_VIRTUAL_SHARED,
        };
        image_t **images = (image_t **)calloc(numImages, sizeof(image_t *));
        // these are NV12 dimenions
        allocator_dimensions_t dims[2] = {{{{1, width,height}}},
                                          {{{2, width/2,height/2}}}};
        ptr_t ptrs[2] = {NULL, NULL};
        value_t hdls[2] = {0,0};
        allocator_dimensions_t strides[2] = {{{{0,0,0}}},
                                             {{{0,0,0}}}};
        allocator_t *alloc = allocator_init();
        uint32_t nptrs = dimof(ptrs);

        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Width=%d Height=%d Num=%d\n", width, height, numImages);
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Alloc=%p Images=%p, Errors=%d\n", alloc, images, errors);

        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "SOSAL_ZONE_MASK=%x\n", SOSAL_ZONE_MASK);

        if (images == NULL)
            errors++;
        if (alloc == NULL)
            errors++;
        if (errors > 0) return errors;

        for (t = 0; t < dimof(types); t++)
        {
            nptrs = dimof(ptrs); // reset value;

#if !defined(SOSAL_USE_TILER) && !defined(SOSAL_USE_ION) && !defined(SOSAL_USE_BO)
            if (types[t] == ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED ||
                types[t] == ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED)
                break;
#endif
#if !defined(SOSAL_USE_GRALLOC) && !defined(SOSAL_USE_BO)
            if (types[t] == ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED)
                break;
#endif
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "IMAGE ALLOC\n");
            for (i = 0; i < numImages; i++) {

                images[i] = image_allocate(width,height,FOURCC_NV12);
                if (images[i] == NULL) {
                    SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to allocate image %d\n", i);
                    errors++;
                }
                SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "PTR CALLOC\n");
                if (allocator_calloc(alloc, types[t], nptrs, 3, dims, ptrs, hdls, strides) == false_e)
                    errors++;
                else
                {
                    for (p = 0; p < nptrs; p++)
                    {
                        fph_set(handle_hash, ptrs[p], (ptr_t)hdls[p]);
                        images[i]->plane[p].ptr = ptrs[p];
                        images[i]->plane[p].xstride = strides[p].dim.x;
                        images[i]->plane[p].ystride = strides[p].dim.y;
                        images[i]->plane[p].numBytes = strides[p].dim.z;
                    }
                }

                SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "IMAGE PRINT\n");
                image_print(images[i]);

            }

            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "IMAGE FREE\n");
            for (i = 0; i < numImages; i++) {

                for (p = 0; p < nptrs; p++)
                {
                    ptrs[p] = (ptr_t)images[i]->plane[p].ptr;
                    fph_get(handle_hash, ptrs[p], (void **)&hdls[p]);
                }

#if defined(SOSAL_USE_TILER)
                nptrs = 1; // tiler buffers only need to free first pointer in set
#endif
                SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "PTR FREE\n");
                if (allocator_free(alloc, types[t], nptrs, ptrs, hdls) == false_e)
                    errors++;

                image_free(&images[i]);
            }
        }

        allocator_deinit(&alloc);
    }
    fph_deinit(handle_hash);
    if (errors == 0)
        return true_e;
    else
    {
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "There were %d errors in allocator unittest!\n", errors);
        return false_e;
    }
}

