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
#include <dvp/dvp_debug.h>

#include <dvp_mem_int.h>
#include <dvp_kgb.h>

typedef struct _dvp_mem_correlate_t {
    allocator_memory_type_e amtype;
    DVP_MemType_e           dmtype;
} DVP_Mem_Correlation_t;

DVP_Mem_Correlation_t mem_correlation[] = {
    {ALLOCATOR_MEMORY_TYPE_VIRTUAL, DVP_MTYPE_MPUCACHED_VIRTUAL},
#if defined(DVP_USE_TILER)
    {ALLOCATOR_MEMORY_TYPE_VIRTUAL, DVP_MTYPE_MPUCACHED_1DTILED},
#endif
#if defined(DVP_USE_ION) || defined(DVP_USE_BO)
    {ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED, DVP_MTYPE_MPUCACHED_1DTILED},
#endif
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    {ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED, DVP_MTYPE_MPUNONCACHED_1DTILED},
    {ALLOCATOR_MEMORY_TYPE_TILED_2D_UNCACHED, DVP_MTYPE_MPUNONCACHED_2DTILED},
#endif
#if defined(DVP_USE_GRALLOC) || defined(DVP_USE_BO)
    {ALLOCATOR_MEMORY_TYPE_GFX_2D_UNCACHED,  DVP_MTYPE_GRALLOC_2DTILED},
#endif
#if defined(DVP_USE_SHARED_T)
    {ALLOCATOR_MEMORY_TYPE_VIRTUAL_SHARED, DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED},
#endif
};

bool_e dvp_mem_type_cached(DVP_MemType_e memType)
{
    switch (memType)
    {
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
        case DVP_MTYPE_MPUCACHED_1DTILED:
            return true_e;
        case DVP_MTYPE_MPUNONCACHED_1DTILED:
        case DVP_MTYPE_MPUNONCACHED_2DTILED:
            return false_e;
#endif
#if defined(DVP_USE_TILER)
        case DVP_MTYPE_DISPLAY_2DTILED:
            return false_e;
#endif
#if defined(DVP_USE_GRALLOC) || defined(DVP_USE_BO)
        case DVP_MTYPE_DISPLAY_2DTILED:
        case DVP_MTYPE_GRALLOC_2DTILED:
            return false_e;
#endif
        default:
            return true_e;
    }
}

allocator_memory_type_e dvp_mem_type_xlate(DVP_MemType_e memType)
{
    DVP_U32 t = 0;
    for (t = 0; t < dimof(mem_correlation); t++)
    {
        if (memType == mem_correlation[t].dmtype)
            return mem_correlation[t].amtype;
    }
    return ALLOCATOR_MEMORY_TYPE_VIRTUAL;
}

void dvp_mem_deinit(DVP_Mem_t **pmem)
{
    allocator_deinit(pmem);
}

DVP_Mem_t *dvp_mem_init()
{
    return allocator_init();
}

DVP_BOOL dvp_mem_flush(DVP_Handle handle,
                       DVP_MemType_e mtype,
                       DVP_S32 nptrs,
                       DVP_S32 ndims,
                       DVP_Dim_t *dims,
                       DVP_PTR *ptrs)
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_S32 p = 0;
    DVP_VALUE hdls[DVP_MAX_PLANES] = {0, 0, 0, 0};
    if (dvp != NULL && nptrs > 0 && ptrs != NULL)
    {
        for (p = 0; p < nptrs; p++)
        {
            hdls[p] = dvp_rpc_get_handle(dvp->rpc, ptrs[p]);
        }

        if (allocator_flush(dvp->mem, dvp_mem_type_xlate(mtype), nptrs, ndims, dims, ptrs, hdls) == false_e)
            return DVP_FALSE;
    }
    return DVP_TRUE;
}

DVP_BOOL dvp_mem_invalidate(DVP_Handle handle,
                            DVP_MemType_e mtype,
                            DVP_S32 nptrs,
                            DVP_S32 ndims,
                            DVP_Dim_t *dims,
                            DVP_PTR *ptrs)
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_S32 p = 0;
    DVP_VALUE hdls[DVP_MAX_PLANES] = {0, 0, 0, 0};
    if (dvp != NULL && nptrs > 0 && ptrs != NULL)
    {
        for (p = 0; p < nptrs; p++)
        {
            hdls[p] = dvp_rpc_get_handle(dvp->rpc, ptrs[p]);
        }

        if (allocator_invalidate(dvp->mem, dvp_mem_type_xlate(mtype), nptrs, ndims, dims, ptrs, hdls) == false_e)
            return DVP_FALSE;
    }
    return DVP_TRUE;
}

DVP_BOOL dvp_mem_free(DVP_Handle handle,
                      DVP_MemType_e mtype,
                      DVP_S32 nptrs,
                      DVP_S32 ndims,
                      DVP_Dim_t *dims,
                      DVP_PTR *ptrs)
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_S32 n = 0, p = 0;
    DVP_U32 size = 1;
    // the handles will be filled-in from the dissociate
    DVP_VALUE hdls[DVP_MAX_PLANES] = {0, 0, 0, 0};

    if (dvp == NULL || nptrs <= 0 || ptrs == NULL || ndims <=0 || dims == NULL)
        return DVP_FALSE;

    for (p = 0; p < nptrs; p++) {

        // calculate the size
        size = 1;
        for (n = 0; n < ndims; n++) {
            size *= dims[p].dims[n];
        }

        // dissociate the memory with the remote cores
        dvp_rpc_dissociate(dvp->rpc, dvp->mem, ptrs[p], &hdls[p], size, mtype);
    }

    if (true_e == allocator_free(dvp->mem, dvp_mem_type_xlate(mtype), nptrs, ptrs, hdls))
        return DVP_TRUE;
    else
        return DVP_FALSE;
}

DVP_BOOL dvp_mem_calloc(DVP_Handle handle,
                        DVP_MemType_e mtype,
                        DVP_S32 nptrs,
                        DVP_S32 ndims,
                        DVP_Dim_t *dims,
                        DVP_PTR *ptrs,
                        DVP_Dim_t *strides)
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_Mem_t *mem = dvp->mem;
    DVP_BOOL ret = DVP_TRUE;
    DVP_U32 size;
    DVP_S32 p;
    // this is filled-in from the allocator and given to associate
    DVP_VALUE hdls[DVP_MAX_PLANES] = {0, 0, 0, 0};

    if (nptrs <= 0 || ndims <= 0 || dims == NULL || ptrs == NULL)
        return DVP_FALSE;

    if (true_e == allocator_calloc(mem, dvp_mem_type_xlate(mtype), nptrs, ndims, dims, ptrs, hdls, strides))
        ret = DVP_TRUE;
    else
        ret = DVP_FALSE;
    if (ret == DVP_TRUE)
    {
        for (p = 0; p < nptrs; p++)
        {
            // use the calculated size of this pointer (this accounts for strides)
            size = strides[p].dims[ndims-1];

            // associate the memory with the remote cores
            dvp_rpc_associate(dvp->rpc, dvp->mem, ptrs[p], hdls[p], size, mtype);
        }
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to allocate memory type %u!\n", mtype);
    }
    return ret;
}

DVP_BOOL dvp_mem_share(DVP_Handle handle,
                       DVP_MemType_e mtype,
                       DVP_S32 nptrs,
                       DVP_PTR *ptrs,
                       DVP_S32 *fds)
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_S32 p = 0;
    DVP_VALUE hdls[DVP_MAX_PLANES] = {0, 0, 0, 0};
    if (dvp != NULL && nptrs > 0 && ptrs != NULL)
    {
        for (p = 0; p < nptrs; p++)
        {
            hdls[p] = dvp_rpc_get_handle(dvp->rpc, ptrs[p]);
        }
        if (allocator_share(dvp->mem, dvp_mem_type_xlate(mtype), nptrs, hdls, fds) == false_e)
            return DVP_FALSE;
    }
    return DVP_TRUE;
}

DVP_BOOL dvp_mem_import(DVP_Handle handle,
                        DVP_MemType_e mtype,
                        DVP_S32 nptrs,
                        DVP_S32 ndims,
                        DVP_Dim_t *dims,
                        DVP_PTR *ptrs,
                        DVP_Dim_t *strides,
                        DVP_S32 *fds,
                        DVP_VALUE *hdls)
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_U32 size;
    DVP_S32 p;
    DVP_BOOL ret = DVP_FALSE;
    if (dvp != NULL && nptrs > 0 && ptrs != NULL && ndims > 0 && dims != NULL)
    {
        if (allocator_import(dvp->mem, dvp_mem_type_xlate(mtype), nptrs, ndims, dims, ptrs, hdls, strides, fds)) {
            ret = DVP_TRUE;
        }
    }
    if (ret == DVP_TRUE && dvp->rpc != NULL) {
        for (p = 0; p < nptrs; p++)
        {
            // use the calculated size of this pointer (this accounts for strides)
            size = strides[p].dims[ndims-1];

            // associate the memory with the remote cores
            dvp_rpc_associate(dvp->rpc, dvp->mem, ptrs[p], hdls[p], size, mtype);
        }
    }
    if (ret == DVP_FALSE) {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to import memory type %u!\n", mtype);
    }
    return ret;
}

DVP_BOOL dvp_mem_free_import(DVP_Handle handle,
                             DVP_MemType_e mtype,
                             DVP_S32 ndims,
                             DVP_Dim_t *dims,
                             DVP_S32 nptrs,
                             DVP_PTR *ptrs,
                             DVP_VALUE *hdls)
{
    DVP_U32 size;
    DVP_S32 n, p;
    DVP_t *dvp = (DVP_t *)handle;

    if (dvp == NULL || nptrs <= 0 || ptrs == NULL || hdls == NULL || ndims <= 0 || dims == NULL)
        return DVP_FALSE;

    if (dvp->rpc != NULL) {
        DVP_VALUE dummyHdls[DVP_MAX_PLANES] = {0, 0, 0, 0};
        for (p = 0; p < nptrs; p++) {
            // calculate the size
            size = 1;
            for (n = 0; n < ndims; n++) {
                size *= dims[p].dims[n];
            }

            // dissociate the memory with the remote cores
            dvp_rpc_dissociate(dvp->rpc, dvp->mem, ptrs[p], &dummyHdls[p], size, mtype);
        }
    }
    if (true_e == allocator_free(dvp->mem, dvp_mem_type_xlate(mtype), nptrs, ptrs, hdls))
        return DVP_TRUE;
    else
        return DVP_FALSE;
}
