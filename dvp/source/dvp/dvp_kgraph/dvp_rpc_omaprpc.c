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

#if defined(DVP_USE_OMAPRPC)

//******************************************************************************
// INCLUDES
//******************************************************************************

#include <sosal/sosal.h>

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>
// @NOTE we include dvp_rpc.h further down in the file...

#ifdef DVP_USE_GRALLOC
#include <hwc/hal_public.h>
#endif

#include <omaprpc/omaprpc.h>

//******************************************************************************
// #DEFINES
//******************************************************************************

#define DVP_RPC_BLOCK_SIZE  (64)
#define DVP_RPC_NUM_BLOCKS  (64)

#define PROC_VALID_ID(x)     (-1 < (x) && (x) < PROC_END)

//******************************************************************************
// TYPEDEFS
//******************************************************************************

#define DVP_CORE_REMOTE_MAX     (DVP_CORE_CPU)

typedef struct _dvp_memory_range_t {
    btreelistnode_t node;
    value_t         base;
    value_t         end;
} DVP_Memory_Range_t;

typedef struct _dvp_rpc_core_t DVP_RPC_Core_t; // forward definition

typedef struct _dvp_rpc_t {
    DVP_Handle handle;         /** This contains the pointer back the DVP_t. It MUST be the FIRST entry in the structure! */
    mutex_t    lock;
    fph_t     *hndl_hash;
    DVP_RPC_Core_t *cores[DVP_CORE_REMOTE_MAX];
    btreelist_t *ranges;
} DVP_RPC_t;

struct _dvp_rpc_core_t {
    DVP_S32 coreEnum;
    omaprpc_t *rpc;
    void *callback; // will be later cast the proper type
    void *cookie;
};

#define DVP_RPC_T   // we're defining the RPC structure locally!
#include <dvp_rpc.h>

#define DVP_VALID_REMOTE_CORE(x)    (DVP_CORE_MIN < (x) && (x) < DVP_CORE_REMOTE_MAX)

typedef struct _dvp_core_to_name_t {
    bool_e enabled;
    DVP_Core_e core;
    char *name;
    char *server_name;
    char *device_name;
} DVP_Core_Name_t;

//******************************************************************************
// LOCAL VARIABLES
//******************************************************************************

#if defined(BLAZE) || defined(SDP) || defined(BLAZE_TABLET) || defined(PANDA)
#define IS_TESLA_ENABLED true_e
#else //p920, GT_I9100G, PLAYBOOK, other platforms
#define IS_TESLA_ENABLED false_e
#endif

#if defined(TARGET_DVP_OMAP4)
static DVP_Core_Name_t dvp_core_names[] = {
    {IS_TESLA_ENABLED,  DVP_CORE_DSP,  "Tesla",  "dvp_kgm_dsp",  "/dev/dvp_kgm_dsp"},
    {true_e,  DVP_CORE_SIMCOP,  "AppM3",  "dvp_kgm_simcop",  "/dev/dvp_kgm_simcop"},
    {false_e, DVP_CORE_MCU0, "AppM3",  "dvp_kgm_app",  "/dev/dvp_kgm_app"},
    {false_e, DVP_CORE_MCU1, "SysM3",  "dvp_kgm_sys",  "/dev/dvp_kgm_sys"},
    {false_e, DVP_CORE_GPU,  "SGX",    "dvp_kgm_sgx",  NULL},
    {false_e, DVP_CORE_EVE,  "EVE",    "dvp_kgm_eve",  NULL},
    {true_e,  DVP_CORE_CPU,  "MPU",    NULL,           NULL},
};
#elif defined(TARGET_DVP_OMAP5)
static DVP_Core_Name_t dvp_core_names[] = {
    {false_e, DVP_CORE_DSP,  "Tesla",  "dvp_kgm_dsp", "/dev/dvp_kgm_dsp"},
    {true_e,  DVP_CORE_SIMCOP,  "AppM4",  "dvp_kgm_simcop", "/dev/dvp_kgm_simcop"},
    {false_e, DVP_CORE_MCU0, "AppM4",  "dvp_kgm_app", "/dev/dvp_kgm_app"},
    {false_e, DVP_CORE_MCU1, "SysM4",  "dvp_kgm_sys", "/dev/dvp_kgm_sys"},
    {false_e, DVP_CORE_GPU,  "SGX",    "dvp_kgm_sgx",  NULL},
    {false_e, DVP_CORE_EVE,  "EVE",    "dvp_kgm_eve",  NULL},
    {true_e,  DVP_CORE_CPU,  "MPU",    NULL,           NULL},
};
#elif defined(TARGET_DVP_OMAP6)
static DVP_Core_Name_t dvp_core_names[] = {
    {true_e,  DVP_CORE_DSP,  "Tesla",  "dvp_kgm_dsp", "/dev/dvp_kgm_dsp"},
    {true_e,  DVP_CORE_SIMCOP,  "AppM4",  "dvp_kgm_simcop", "/dev/dvp_kgm_simcop"},
    {false_e, DVP_CORE_MCU0, "AppM4",  "dvp_kgm_app", "/dev/dvp_kgm_app"},
    {false_e, DVP_CORE_MCU1, "SysM4",  "dvp_kgm_sys", "/dev/dvp_kgm_sys"},
    {false_e, DVP_CORE_GPU,  "SGX",    "dvp_kgm_sgx",  NULL},
    {false_e, DVP_CORE_EVE,  "EVE",    "dvp_kgm_eve",  "/dev/omaprpc-eve"},
    {true_e,  DVP_CORE_CPU,  "MPU",    NULL,           NULL},
};
#endif // OMAP types

static DVP_Perf_t rpc_perf;

//******************************************************************************
// LOCAL FUNCTIONS
//******************************************************************************

static bool_e dvp_rpc_core_is_enabled(DVP_Core_e core)
{
    DVP_U32 i = 0;
    for (i = 0; i < DVP_CORE_MAX; i++)
        if (core == dvp_core_names[i].core)
            return dvp_core_names[i].enabled;
    return false_e;
}

/** This is a handy internal MACRO used to increment over all the cores which are
 * valid AND enabled. This should be used STRICTLY as a FOR loop replacement!
 */
#define DVP_FOR_EACH_CORE(c)    for (c = (DVP_CORE_MIN+1); c < DVP_CORE_REMOTE_MAX; c++)

static int dvp_mem_range_sort(btreelistnode_t *node, btreelistnode_t *obj)
{
    DVP_Memory_Range_t *r = (DVP_Memory_Range_t *)node;
    DVP_Memory_Range_t *s = (DVP_Memory_Range_t *)obj;

    if (s->end <= r->base)
        return -1;
    else if (r->end <= s->base)
        return 1;
    else // any overlap
        return 0;
}

static int dvp_mem_range_compare(btreelistnode_t *node, value_t value)
{
    DVP_Memory_Range_t *r = (DVP_Memory_Range_t *)node;

    if (r->base <= value && value < r->end)
        return 0;
    else if (value < r->base)
        return -1;
    else // if (value >= r->end)
        return 1;
}

static char *dvp_mem_range_print(btreelistnode_t *node)
{
    static char values[255];
    DVP_Memory_Range_t *r = (DVP_Memory_Range_t *)node;
    sprintf(values, "base "FMT_VALUE_T" end "FMT_VALUE_T"", r->base, r->end);
    return values;
}

//******************************************************************************
// GLOBAL FUNCTIONS
//******************************************************************************

void dvp_rpc_mem_remove(DVP_RPC_t *rpc, DVP_PTR localAddr, DVP_U32 localSize, DVP_MemType_e memType)
{
    rpc = rpc;
    localAddr = localAddr;
    localSize = localSize;
    memType = memType;
}

DVP_PTR dvp_rpc_mem_xlate_fwrd(DVP_RPC_t *rpc, DVP_Core_e core, DVP_PTR localAddr, DVP_U32 localSize, DVP_MemType_e memType)
{
    DVP_PTR remoteAddr = NULL;
    rpc = rpc; // remove warning
    core = core;// remove warning
    localSize = localSize; // remove warning
    memType = memType; // remove warning
    // when RCM is off, everything is local (kernel does OMAPRPC translations)
    remoteAddr = localAddr;
    return remoteAddr;
}

DVP_PTR dvp_rpc_mem_xlate_back(DVP_RPC_t *rpc, DVP_Core_e core, DVP_PTR remoteAddr, DVP_MemType_e memType)
{
    DVP_PTR localAddr = NULL;
    rpc = rpc; // remove warning
    core = core; // remove warning
    memType = memType; // remove warning
    // when RCM is off, everything is local (kernel does OMAPRPC translations)
    localAddr = remoteAddr;
    return localAddr;
}

DVP_BOOL dvp_rpc_flush(DVP_RPC_t *rpc, DVP_Core_e core __attribute__((unused)), DVP_PTR address, DVP_U32 size, DVP_MemType_e mtype)
{
    DVP_BOOL ret = DVP_FALSE;
    if (rpc)
    {
        DVP_PRINT(DVP_ZONE_RPC, "Flushing address:%p for %u bytes type:%d core:%d\n", address, size, mtype, core);
        DVP_Dim_t dims[1] = {{{{1,size,1}}}};
        DVP_PTR   ptrs[1] = {(DVP_PTR)address};
        core = DVP_CORE_MIN;
        size = 0;
        // 1 pointer, 2 dimensions (bpp and width so to speak)
        ret = dvp_mem_flush(rpc->handle, mtype, dimof(ptrs), 2, dims, ptrs);
    }
    return ret;
}

DVP_BOOL dvp_rpc_invalidate(DVP_RPC_t *rpc, DVP_Core_e core __attribute__((unused)), DVP_PTR address, DVP_U32 size, DVP_MemType_e mtype)
{
    DVP_BOOL ret = DVP_FALSE;
    if (rpc)
    {
        DVP_PRINT(DVP_ZONE_RPC, "Invalidating address:%p for %u bytes type:%d core:%d\n", address, size, mtype, core);
        DVP_Dim_t dims[1] = {{{{1,size,1}}}};
        DVP_PTR   ptrs[1] = {(DVP_PTR)address};
        core = DVP_CORE_MIN;
        size = 0;
        ret = dvp_mem_invalidate(rpc->handle, mtype, dimof(ptrs), 2, dims, ptrs);
    }
    return ret;
}

void dvp_rpc_prepare_image(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Image_t *pImage, DVP_BOOL cacheOp, DVP_PTR base, DVP_RPC_Translation_t *trans)
{
    DVP_U32 p = 0;
    DVP_BOOL cached = DVP_TRUE;

    // Remove warnings
    base = base;
    trans = trans;

    if (trans == NULL)
        return;

#if defined(DVP_USE_GRALLOC) || defined(DVP_USE_BO)
    if (pImage->memType == DVP_MTYPE_DISPLAY_2DTILED)
        cached = DVP_FALSE;
#endif
#if defined(DVP_USE_ION) || defined(DVP_USE_BO)
    if (pImage->memType == DVP_MTYPE_MPUNONCACHED_1DTILED ||
        pImage->memType == DVP_MTYPE_MPUNONCACHED_2DTILED)
        cached = DVP_FALSE;
#endif
#if defined(DVP_USE_GRALLOC)
    if (pImage->memType == DVP_MTYPE_GRALLOC_2DTILED)
        cached = DVP_FALSE;
#endif
    for (p = 0; p < pImage->planes; p++)
    {
        DVP_U32 planeSize = DVP_Image_PlaneSize(pImage, p);
        if (cached && cacheOp && !pImage->skipCacheOpFlush) {
            DVP_COMPLAIN_IF_FALSE(dvp_rpc_flush(rpc, core, pImage->pBuffer[p], planeSize, pImage->memType));
        }

        {
            ptr_t handle = NULL;
            core = core; // @TODO put the desired core into the translation structure to speed up lookups
            fph_get(rpc->hndl_hash, (ptr_t)pImage->pBuffer[p], &handle);
            trans->translations[trans->numTranslations].cacheops = cacheOp;
            trans->translations[trans->numTranslations].offset = ((DVP_U32)&pImage->pData[p] - (DVP_U32)base);
            trans->translations[trans->numTranslations].base = (DVP_U32)pImage->pBuffer[p];
#if defined(DVP_USE_GRALLOC)
            if (pImage->memType == DVP_MTYPE_DISPLAY_2DTILED ||
                pImage->memType == DVP_MTYPE_GRALLOC_2DTILED)
            {
                // gralloc fd needs to be passed down
                trans->translations[trans->numTranslations].reserved = (DVP_PTR)(((IMG_native_handle_t *)pImage->reserved)->fd[p]);
            }
            else
#elif defined(DVP_USE_BO)
            if (pImage->memType == DVP_MTYPE_MPUCACHED_1DTILED ||
                pImage->memType == DVP_MTYPE_MPUNONCACHED_1DTILED ||
                pImage->memType == DVP_MTYPE_MPUNONCACHED_2DTILED ||
                pImage->memType == DVP_MTYPE_DISPLAY_2DTILED)
            {
                // give the driver the file descriptor of the DMA BUF (each plane has it's own fd).
                trans->translations[trans->numTranslations].reserved = (DVP_PTR)(((alloc_omap_bo_t *)handle)->dmabuf_fd);
            }
            else
#endif
            {
                trans->translations[trans->numTranslations].reserved = handle;
            }
            DVP_PRINT(DVP_ZONE_RPC, "+[%u] TRANS BASE:%p OFF:0x%08x DATA BASE:%p DATA:%p RESV:%p\n",
                      trans->numTranslations,
                      base,
                      trans->translations[trans->numTranslations].offset,
                      (DVP_PTR)pImage->pBuffer[p],
                      (DVP_PTR)pImage->pData[p],
                      trans->translations[trans->numTranslations].reserved);
            trans->numTranslations++;
        }
     }
}

void dvp_rpc_return_image(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Image_t *pImage, DVP_BOOL cacheOp)
{
    DVP_BOOL cached = DVP_TRUE;
    DVP_U32 p = 0;

 #if defined(DVP_USE_GRALLOC) || defined(DVP_USE_BO)
    if (pImage->memType == DVP_MTYPE_DISPLAY_2DTILED)
        cached = DVP_FALSE;
#endif
#if defined(DVP_USE_ION) || defined(DVP_USE_BO)
    if (pImage->memType == DVP_MTYPE_MPUNONCACHED_1DTILED ||
        pImage->memType == DVP_MTYPE_MPUNONCACHED_2DTILED)
        cached = DVP_FALSE;
#endif
#if defined(DVP_USE_GRALLOC)
    if (pImage->memType == DVP_MTYPE_GRALLOC_2DTILED)
        cached = DVP_FALSE;
#endif
    for (p = 0; p < pImage->planes; p++)
    {
        DVP_U32 planeSize = DVP_Image_PlaneSize(pImage, p);
        pImage->pBuffer[p] = dvp_rpc_mem_xlate_back(rpc, core, pImage->pBuffer[p], pImage->memType);
        if (cached && cacheOp && !pImage->skipCacheOpInval) {
            DVP_COMPLAIN_IF_FALSE(dvp_rpc_invalidate(rpc, core, (DVP_PTR)pImage->pBuffer[p], planeSize, pImage->memType));
        }
    }
}

void dvp_rpc_prepare_buffer(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Buffer_t *pBuffer, DVP_BOOL cacheOp, DVP_PTR base, DVP_RPC_Translation_t *trans)
{
    DVP_BOOL cached = DVP_TRUE;

    // Remove warnings
    base = base;
    trans = trans;

#if defined(DVP_USE_ION) || defined(DVP_USE_BO)
    if (pBuffer->memType == DVP_MTYPE_MPUNONCACHED_1DTILED)
        cached = DVP_FALSE;
#endif
    if (cached && cacheOp && !pBuffer->skipCacheOpFlush) {
        DVP_COMPLAIN_IF_FALSE(dvp_rpc_flush(rpc, core, pBuffer->pData, pBuffer->numBytes, pBuffer->memType));
    }

    if (trans)
    {
        ptr_t handle = NULL;
        core = core; // @TODO put core into translation structure
        fph_get(rpc->hndl_hash, (ptr_t)pBuffer->pData, &handle);
        trans->translations[trans->numTranslations].cacheops = cacheOp;
        trans->translations[trans->numTranslations].offset = ((DVP_U32)&pBuffer->pData - (DVP_U32)base);
        trans->translations[trans->numTranslations].base = (DVP_U32)pBuffer->pData;
#if defined(DVP_USE_BO)
        trans->translations[trans->numTranslations].reserved = (DVP_PTR)(((alloc_omap_bo_t *)handle)->dmabuf_fd);
#else
        trans->translations[trans->numTranslations].reserved = handle;
#endif
        trans->numTranslations++;
    }
}

void dvp_rpc_return_buffer(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Buffer_t *pBuffer, DVP_BOOL cacheOp)
{
    DVP_BOOL cached = DVP_TRUE;
#if defined(DVP_USE_ION) || defined(DVP_USE_BO)
    if (pBuffer->memType == DVP_MTYPE_MPUNONCACHED_1DTILED)
        cached = DVP_FALSE;
#endif
    pBuffer->pData = dvp_rpc_mem_xlate_back(rpc, core, pBuffer->pData, pBuffer->memType);
    if (cached && cacheOp && !pBuffer->skipCacheOpInval) {
        DVP_COMPLAIN_IF_FALSE(dvp_rpc_invalidate(rpc, core, (DVP_PTR)pBuffer->pData, pBuffer->numBytes, pBuffer->memType));
    }
}

DVP_VALUE dvp_rpc_get_handle(DVP_RPC_t *rpc, DVP_PTR ptr)
{
    DVP_VALUE handle = 0;
    DVP_VALUE value = 0;
    if (rpc != NULL && ptr != NULL)
    {
        // check the ranges first for the base pointer
        DVP_Memory_Range_t *mrange = (DVP_Memory_Range_t *)btreelist_find(rpc->ranges, (value_t)ptr);
        if (mrange) {
            DVP_PRINT(DVP_ZONE_MEM, "PTR:%p has BASE:%p, replacing in query!\n", ptr, (ptr_t)mrange->base);
            ptr = (DVP_PTR)mrange->base;
        }
        if (fph_get(rpc->hndl_hash, ptr, (ptr_t *)&handle))
        {
            DVP_PRINT(DVP_ZONE_MEM, "PTR:%p => HDL:"FMT_VALUE_T"\n", ptr, handle);
#if defined(DVP_USE_BO)
            value = ((alloc_omap_bo_t *)handle)->dmabuf_fd;
#elif defined(DVP_USE_ION)
            value = handle;
#endif
        }
        else
        {
            DVP_PRINT(DVP_ZONE_WARNING, "PTR:%p was not in handle hash! (this may be ok)\n", ptr);
        }
    }
    return value;
 }

DVP_BOOL dvp_rpc_dissociate(DVP_RPC_t *rpc, DVP_Mem_t *mem, DVP_PTR ptr, DVP_VALUE *pHandle, size_t size, DVP_MemType_e mtype)
{
    DVP_BOOL ret = DVP_TRUE;

    if (rpc == NULL || mem == NULL || ptr == NULL || size == 0)
        return DVP_FALSE;

    switch (mtype)
    {
#if defined(DVP_USE_ION) || defined(DVP_USE_BO)
        case DVP_MTYPE_MPUCACHED_1DTILED:
        case DVP_MTYPE_MPUNONCACHED_1DTILED:
        case DVP_MTYPE_MPUNONCACHED_2DTILED:
#endif
#if defined(DVP_USE_GRALLOC)
        case DVP_MTYPE_GRALLOC_2DTILED:
        case DVP_MTYPE_DISPLAY_2DTILED:
#endif
        case DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED:
#if defined(DVP_USE_CAMERA_SERVICE)
        case DVP_MTYPE_CAMERA_1DTILED:
#endif
        {
            DVP_Memory_Range_t *mrange = NULL;
            mutex_lock(&rpc->lock);
            if (fph_get(rpc->hndl_hash, ptr, (ptr_t *)pHandle)) {
                // pHandle is now set
                DVP_Core_e core;
                // clear out the handle
                fph_set(rpc->hndl_hash, ptr, NULL);
                // unregister from each core
                if (mtype == DVP_MTYPE_MPUNONCACHED_1DTILED ||
                    mtype == DVP_MTYPE_MPUNONCACHED_2DTILED ||
                    mtype == DVP_MTYPE_MPUCACHED_1DTILED)
                {
#if defined(SOSAL_USE_ION) && defined(DVP_USE_ION) && defined(OMAPRPC_USE_ION)
                    DVP_FOR_EACH_CORE(core) {
                        // if the core is enabled and actually is running
                        if (dvp_core_names[core].enabled && rpc->cores[core]) {
                            omaprpc_unregister(rpc->cores[core]->rpc, 0, ptr, (void **)pHandle);
                        }
                    }
#endif
                }
                // remove the range from the tree
                mrange = (DVP_Memory_Range_t *)btreelist_remove(rpc->ranges, (value_t)ptr);
                if (mrange)
                    free(mrange);
            }
            else
                ret = DVP_FALSE;
            mutex_unlock(&rpc->lock);
            break;
        }
        case DVP_MTYPE_MPUCACHED_VIRTUAL:
        default:
            ret = DVP_FALSE;
            break;
    }
    if (ret == DVP_TRUE) {
        DVP_PRINT(DVP_ZONE_RPC, "Dissociated %p of "FMT_SIZE_T" bytes (hdl: "FMT_VALUE_T")\n", ptr, size, *pHandle);
    } else {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to dissociate %p of "FMT_SIZE_T" bytes (hdl: "FMT_VALUE_T")\n", ptr, size, *pHandle);
    }
    return ret;
}

DVP_BOOL dvp_rpc_associate(DVP_RPC_t *rpc, DVP_Mem_t *mem, DVP_PTR ptr, DVP_VALUE handle, size_t size, DVP_MemType_e mtype)
{
    DVP_BOOL ret = DVP_TRUE;

    if (rpc == NULL || mem == NULL || ptr == NULL || size == 0)
        return DVP_FALSE;

    DVP_PRINT(DVP_ZONE_RPC, "Associating PTR:%p HDL:"FMT_VALUE_T" SIZE:%zu TYPE:%d\n", ptr, handle, size, mtype);
    switch (mtype)
    {
#if defined(DVP_USE_ION) || defined(DVP_USE_BO)
        case DVP_MTYPE_MPUCACHED_1DTILED: // not supported on ICS
        case DVP_MTYPE_MPUNONCACHED_1DTILED:
        case DVP_MTYPE_MPUNONCACHED_2DTILED:
#endif
#if defined(DVP_USE_GRALLOC)
        case DVP_MTYPE_GRALLOC_2DTILED:
        case DVP_MTYPE_DISPLAY_2DTILED:   // not supported on ICS
#endif
        case DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED:
#if defined(DVP_USE_CAMERA_SERVICE)
        case DVP_MTYPE_CAMERA_1DTILED:
#endif
        {
            DVP_Memory_Range_t *mrange = (DVP_Memory_Range_t *)calloc(1, sizeof(DVP_Memory_Range_t));
            if (mrange)
            {
                mrange->base = (uint32_t)ptr;
                mrange->end = (uint32_t)ptr + (uint32_t)size;
            }
            mutex_lock(&rpc->lock);
            btreelist_add(rpc->ranges, (btreelistnode_t *)mrange);
            if (fph_set(rpc->hndl_hash, (ptr_t)ptr, (ptr_t)handle) == 0)
            {
                // failed
                mrange = (DVP_Memory_Range_t *)btreelist_remove(rpc->ranges, (value_t)ptr);
                free(mrange);
            }
            else if (mtype == DVP_MTYPE_MPUCACHED_1DTILED ||
                     mtype == DVP_MTYPE_MPUNONCACHED_1DTILED ||
                     mtype == DVP_MTYPE_MPUNONCACHED_2DTILED)
            {
#if defined(SOSAL_USE_ION) && defined(DVP_USE_ION) && defined(OMAPRPC_USE_ION)
                DVP_Core_e core;
                DVP_FOR_EACH_CORE(core) {
                    if (dvp_core_names[core].enabled && rpc->cores[core]) {
                        int fd = allocator_get_device(mem, dvp_mem_type_xlate(mtype));
                        omaprpc_register(rpc->cores[core]->rpc, fd, ptr, (void **)&handle);
                    }
                }
#endif
            }
            mutex_unlock(&rpc->lock);
            break;
        }
        case DVP_MTYPE_MPUCACHED_VIRTUAL: // not supported on ICS
        default: // not supported
            ret = DVP_FALSE;
            break;
    }
    return ret;
}

void dvp_rpc_deinit(DVP_RPC_t **rpc)
{
    if (rpc && *rpc)
    {
        btreelist_destroy((*rpc)->ranges);
        fph_deinit((*rpc)->hndl_hash);
        mutex_deinit(&(*rpc)->lock);
        free(*rpc);
        *rpc = NULL;
    }
}


DVP_RPC_t *dvp_rpc_init()
{
    DVP_RPC_t *rpc = (DVP_RPC_t *)calloc(1,sizeof(DVP_RPC_t));
    if (rpc)
    {
        mutex_init(&rpc->lock);
        rpc->hndl_hash = fph_init(DVP_RPC_NUM_BLOCKS, DVP_RPC_BLOCK_SIZE, NULL);
        rpc->ranges = btreelist_create(dvp_mem_range_compare, dvp_mem_range_sort, dvp_mem_range_print);
    }
    DVP_Perf_Clear(&rpc_perf);
    return rpc;
}

void dvp_rpc_core_deinit(DVP_RPC_t *rpc, DVP_RPC_Core_t **rpcc)
{
    if ( rpcc && *rpcc )
        omaprpc_close(&(*rpcc)->rpc);

    if (rpc && rpcc && *rpcc)
    {
        free(*rpcc);
        *rpcc = NULL;
    }
}

static void dvp_rpc_restart_callback(omaprpc_t *rpc __attribute__((unused)), void *cookie)
{
    DVP_RPC_Core_t *rpcc = (DVP_RPC_Core_t *)cookie;
    if (rpcc && rpcc->callback)
    {
        // this is typecast due to the way we define the DVP_RPC_Core_t structure.
        dvp_rpc_restart_f callback = (dvp_rpc_restart_f)rpcc->callback;

        DVP_PRINT(DVP_ZONE_RPC, "RESTART Callback issued to %p\n", rpcc->callback);
        callback(rpcc->cookie);
    }
}

void dvp_rpc_restart_callback_register(DVP_RPC_t *rpc,
                                        DVP_RPC_Core_t *rpcc,
                                        dvp_rpc_restart_f func,
                                        void *cookie)
{
    if (rpc && rpcc && func)
    {
        rpcc->callback = (void *)func;
        rpcc->cookie = cookie;
    }
}

DVP_RPC_Core_t *dvp_rpc_core_init(DVP_RPC_t *rpc, DVP_RPC_Interface_t *rpci)
{
    DVP_RPC_Core_t *rpcc = NULL;
    int32_t status = 0;

    if (rpc == NULL || rpci == NULL)
        return NULL;

    rpcc = (DVP_RPC_Core_t *)calloc(1,sizeof(DVP_RPC_Core_t));
    if (rpcc)
    {
        DVP_Core_e core = rpci->coreEnum;
        rpcc->coreEnum = core;
        if (DVP_VALID_REMOTE_CORE(core) && dvp_rpc_core_is_enabled(core))
        {
            DVP_PRINT(DVP_ZONE_RPC, "DVP_RPC: Interfacing to CORE:%d\n",core);
            rpcc->rpc = omaprpc_open(dvp_core_names[core].device_name, dvp_core_names[core].server_name, rpci->numRemoteFunctions);

            if (rpcc->rpc == NULL)
            {
                status = -1;
                DVP_PRINT(DVP_ZONE_RPC, "DVP_RPC: CORE:%u is disabled!\n", core);
                dvp_core_names[core].enabled = false_e;
            }
            else
            {
                status = 0;
                dvp_core_names[core].enabled = true_e;
                rpc->cores[core] = rpcc;
                omaprpc_restart_callback(rpcc->rpc, rpcc, dvp_rpc_restart_callback, DVP_MAX_RESTART_RETRY);
            }
            if (status < 0)
            {
                goto leave;
            }
            DVP_PRINT(DVP_ZONE_RPC, "Successfully installed DVP RPC for CORE %d!\n",core);
        }
        else
        {
            status = -1; // not a valid core
        }
    }
leave:
    if (status < 0) // if ANY failure!
    {
        dvp_rpc_core_deinit(rpc, &rpcc);
    }
    return rpcc;
}

DVP_S32 dvp_rpc_remote_execute(DVP_RPC_t *rpc,
                               DVP_RPC_Core_t *rpcc,
                               DVP_U32 cliIndex,
                               DVP_RPC_Parameter_t *params,
                               DVP_U32 numParams,
                               DVP_RPC_Translation_t *trans)
{
    int ret = 0;

    if (rpc == NULL || rpcc == NULL)
        return 0;

    // trans can be NULL if there is nothing to translate
    // remove warnings
    trans = trans;

    DVP_U32 numTranslations = 0;
    DVP_U32 i, size = 0;
    struct omaprpc_call_function_t *function = NULL;
    struct omaprpc_function_return_t returned;

    if (numParams > OMAPRPC_MAX_PARAMETERS)
        return -1;

    if (trans)
        numTranslations = trans->numTranslations;

    DVP_PerformanceStart(&rpc_perf);

    // compute the size the structure plus the number of translations times the
    // size of a translation.
    size = sizeof(struct omaprpc_call_function_t);
    size += (numTranslations * sizeof(struct omaprpc_param_translation_t));

    DVP_PRINT(DVP_ZONE_RPC, "Allocating %u bytes for Function Call\n", size);

    // allocate the function structure
    function = (struct omaprpc_call_function_t *)calloc(1, size);
    if (function)
    {
        // transfer the parameter structures over to the OMAPRPC mechanisms
        function->func_index = cliIndex;
        function->num_params = numParams;
        for (i = 0; i < numParams; i++)
        {
            DVP_PRINT(DVP_ZONE_RPC, "PARAM[%u] TYPE: %u DATA: 0x%x SIZE: %u RESV: %p\n",
                i, params[i].type, *(DVP_U32 *)params[i].data, params[i].size, params[i].reserved);
            function->params[i].size = params[i].size;
            if (params[i].reserved)
            {
                DVP_Memory_Range_t *r = NULL;
                function->params[i].type = OMAPRPC_PARAM_TYPE_PTR;
                function->params[i].data = (uint32_t)*(DVP_PTR *)params[i].data;
                function->params[i].base = (uint32_t)*(DVP_PTR *)params[i].data; // default to same value
                r = (DVP_Memory_Range_t *)btreelist_find(rpc->ranges, *(value_t *)params[i].data);
                if (r)
                {
                    DVP_PRINT(DVP_ZONE_RPC, "Found Data %p base address %p\n", *(DVP_PTR *)params[i].data, (DVP_PTR)r->base);
                    function->params[i].base = r->base;
                }
                function->params[i].reserved = (uint32_t)params[i].reserved;
            }
            else
            {
                switch (params[i].size)
                {
                    case sizeof(DVP_U08):
                        function->params[i].type = OMAPRPC_PARAM_TYPE_ATOMIC;
                        function->params[i].data = (uint32_t)*(DVP_U08 *)params[i].data;
                        break;
                    case sizeof(DVP_U16):
                        function->params[i].type = OMAPRPC_PARAM_TYPE_ATOMIC;
                        function->params[i].data = (uint32_t)*(DVP_U16 *)params[i].data;
                        break;
                    case sizeof(DVP_U32):
                        function->params[i].type = OMAPRPC_PARAM_TYPE_ATOMIC;
                        function->params[i].data = *(DVP_U32 *)params[i].data;
                        break;
                    default:
                        function->params[i].type = OMAPRPC_PARAM_TYPE_STRUCT;
                        function->params[i].data = 0; // this is not supported yet.
                        DVP_PRINT(DVP_ZONE_ERROR, "Inside an unsupported case!!\n");
                        break;
                }
            }
        }
        if (trans && trans->numTranslations > 0)
        {
            function->num_translations = trans->numTranslations;
            for (i = 0; i < trans->numTranslations; i++)
            {
                DVP_PTR ptr = *(DVP_PTR*)(function->params[trans->translations[i].index].data + trans->translations[i].offset);
                ptr = ptr; // warnings
                DVP_PRINT(DVP_ZONE_RPC, "TRANS[%04u] IDX:%u DATA:%p PTR:%p RESV:%p OFF:%u (0x%08x) \n",
                          i, trans->translations[i].index,
                          (DVP_PTR)function->params[trans->translations[i].index].data,
                          ptr,
                          trans->translations[i].reserved,
                          trans->translations[i].offset,
                          trans->translations[i].offset);
                function->translations[i].index = trans->translations[i].index;
                function->translations[i].offset = trans->translations[i].offset;
                function->translations[i].base = trans->translations[i].base;
                function->translations[i].cacheOps = trans->translations[i].cacheops;
                function->translations[i].reserved = (uint32_t)trans->translations[i].reserved; // @TODO bit depth problem on 64 bit platform!
            }
        }
#if DVP_DEBUG == 1
        fflush(stdout); // make sure all the prints are flushed before we call the kernel, else we'll lose some debugging.
#endif
        if (omaprpc_call(rpcc->rpc, function, &returned) == false_e)
        {
            DVP_PRINT(DVP_ZONE_ERROR, "OMAPRPC Call failed!\n");
            ret = -1;
        }
        else
            ret = returned.status;

        free(function);
    }
    DVP_PerformanceStop(&rpc_perf);
    DVP_PerformancePrint(&rpc_perf, "DVP RPC EXEC");
    return ret;

}

#endif

//******************************************************************************
// END
//******************************************************************************
