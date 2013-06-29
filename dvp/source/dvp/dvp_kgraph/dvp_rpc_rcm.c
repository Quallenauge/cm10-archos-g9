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

#if defined(DVP_USE_RCM)

//******************************************************************************
// INCLUDES
//******************************************************************************

#include <sosal/sosal.h>

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>
// @NOTE we include dvp_rpc.h further down in the file...

// RCM/Syslink Includes
#include <Std.h>
#include <IpcUsr.h>
#include <ProcMgr.h>
#include <RcmClient.h>
#include <RcmServer.h>

#ifdef DVP_USE_TILER
#include <memmgr.h>
#include <SysLinkMemUtils.h>
#endif

//******************************************************************************
// #DEFINES
//******************************************************************************

#define DVP_RPC_BLOCK_SIZE  (64)
#define DVP_RPC_NUM_BLOCKS  (64)

#define PROC_VALID_ID(x)     (-1 < (x) && (x) < PROC_END)

#define RCM_GOTO_IF_FAILED(s, statement, label) { \
    s = statement;\
    if (s < 0) { \
        printf(""#statement" failed (%d) at %s:%u\n", s,__FILE__, __LINE__);\
        goto label; \
    }\
}

#define RCM_COMPLAIN_IF_FAILED(s, statement) { \
    s = statement;\
    if (s < 0) { \
        printf(""#statement" failed (%d) at %s:%u\n", s, __FILE__, __LINE__);\
    }\
}

//******************************************************************************
// TYPEDEFS
//******************************************************************************

#define DVP_MAX_SERVER_NAME     (20)
typedef struct _dvp_proc_t {
    UInt16 id; /**< Used to contain the actual ID of the core, use dvp_rpc_getid() for the functional id */
    DVP_U32 entry;
    ProcMgr_Handle handle;
    ProcMgr_AttachParams params;
} DVP_Proc_t;

typedef struct _dvp_server_t {
    char name[DVP_MAX_SERVER_NAME];
    RcmServer_Params params;
    RcmServer_Handle handle;
    struct _dvp_function_t *calls;
    DVP_U32 numCalls;
} DVP_Server_t;

typedef struct _dvp_client_t {
    char name[DVP_MAX_SERVER_NAME];
    RcmClient_Params params;
    RcmClient_Handle handle;
    struct _dvp_function_t *calls;
    DVP_U32 numCalls;
} DVP_Client_t;

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
    Ipc_Config config;
    DVP_Proc_t procs[DVP_CORE_REMOTE_MAX];
    fph_t     *mpu2rpu_hash[DVP_CORE_REMOTE_MAX];
    fph_t     *rpu2mpu_hash[DVP_CORE_REMOTE_MAX];
#if defined(DVP_USE_TILER)
    fph_t     *tiler2rpu_hash[DVP_CORE_REMOTE_MAX];
    fph_t     *rpu2tiler_hash[DVP_CORE_REMOTE_MAX];
    fph_t     *tiler2mpu_hash;
    fph_t     *mpu2tiler_hash;
#endif
} DVP_RPC_t;

struct _dvp_rpc_core_t {
    DVP_S32 coreEnum;
    DVP_Server_t server;
    DVP_Client_t client;
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
    char *client_name;
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
    {IS_TESLA_ENABLED,  DVP_CORE_DSP,  "Tesla",  "DVPSrv_ChironDSP",  "DVPSrv_Tesla"},
    {true_e,  DVP_CORE_SIMCOP,  "AppM3",  "DVPSrv_ChironSIMCOP",  "DVPSrv_Ducati1"},
    {false_e, DVP_CORE_MCU0, "AppM3",  "DVPSrv_ChironMCU0", "DVPSrv_Ducati2"},
    {false_e, DVP_CORE_MCU1, "SysM3",  "DVPSrv_ChironMCU1", "DVPSrv_Ducati1"},
    {false_e, DVP_CORE_GPU,  "SGX",    "DVPSrv_ChironSGX",  NULL},
    {false_e, DVP_CORE_EVE,  "EVE",    "DVPSrv_ChironEVE",  NULL},
    {true_e,  DVP_CORE_CPU,  "MPU",    NULL,                NULL},
};
#endif // OMAP types

static DVP_Perf_t rpc_perf;

//******************************************************************************
// LOCAL FUNCTIONS
//******************************************************************************

static ProcMgr_ProcId dvp_rpc_getid(DVP_Core_e core)
{
    switch (core)
    {
#if defined(TARGET_DVP_OMAP4)
        case DVP_CORE_DSP:
            return PROC_TESLA;
        case DVP_CORE_SIMCOP:
        case DVP_CORE_MCU0:
        case DVP_CORE_MCU1:
            return PROC_SYSM3;
#endif
        case DVP_CORE_CPU:
        default:
            return PROC_MPU;
    }
}

static DVP_Core_e dvp_rpc_next_core(DVP_Core_e core)
{
    if (!DVP_VALID_CORE(core))
        core = DVP_CORE_MIN;
    do {
        core += 1;
    } while (core < DVP_CORE_MAX && dvp_core_names[core].enabled == false_e);
    return core;
}

static bool_e dvp_rpc_core_is_enabled(DVP_Core_e core)
{
    DVP_U32 i = 0;
    for (i = 0; i < DVP_CORE_MAX; i++)
        if (core == dvp_core_names[i].core)
            return dvp_core_names[i].enabled;
    return false_e;
}

static int dvp_rpc_server_deinit(DVP_Server_t *s)
{
    Int status = 0;
    RCM_COMPLAIN_IF_FAILED(status, RcmServer_delete(&s->handle));
    RcmServer_exit();
    return status;
}

static int dvp_rpc_server_init(DVP_Server_t *s)
{
    DVP_U32 f = 0;
    Int status = 0;
    RcmServer_init();
    RCM_GOTO_IF_FAILED(status, RcmServer_Params_init(&s->params),leave);
    RCM_GOTO_IF_FAILED(status, RcmServer_create(s->name, &s->params, &s->handle),leave);
    for (f = 0; f < s->numCalls; f++) {
        RCM_GOTO_IF_FAILED(status, RcmServer_addSymbol(s->handle, s->calls[f].name, (RcmServer_MsgFxn)s->calls[f].function, &s->calls[f].index),leave);
        DVP_PRINT(DVP_ZONE_RPC, "%s[%u] %s\n", s->name, s->calls[f].index, s->calls[f].name);
    }
    RcmServer_start(s->handle);
leave:
    //if (status < 0)
    //    dvp_rpc_server_deinit(s);
    return status;
}

static int dvp_rpc_client_deinit(DVP_Client_t *c)
{
    Int status = 0;
    RCM_COMPLAIN_IF_FAILED(status, RcmClient_delete(&c->handle));
    RcmClient_exit();
    return status;
}

static int dvp_rpc_client_init(DVP_Client_t *c)
{
    DVP_U32 i = 0;
    Int status = 0;
    RcmClient_init();
    RCM_GOTO_IF_FAILED(status, RcmClient_Params_init(&c->params),leave);
    c->params.heapId = 1; /// @todo Determine which heap is this exactly.
    DVP_PRINT(DVP_ZONE_RPC, "Attempting to connect to remote server %s\n", c->name);
    RCM_GOTO_IF_FAILED(status, RcmClient_create(c->name, &c->params, &c->handle),leave);
    DVP_PRINT(DVP_ZONE_RPC, "Connected to remote server %s\n", c->name);
    DVP_PRINT(DVP_ZONE_RPC, "Attempting to get the indexes of %u functions!\n", c->numCalls);
    for (i = 0; i < c->numCalls; i++)
    {
        RCM_GOTO_IF_FAILED(status, RcmClient_getSymbolIndex(c->handle, c->calls[i].name, &c->calls[i].index), leave);
        DVP_PRINT(DVP_ZONE_RPC, "REMOTE %s::%s[%u]\n",c->name, c->calls[i].name, c->calls[i].index);
    }
leave:
    //if (status < 0)
    //    dvp_rpc_client_deinit(c);
    return status;
}


/** This is a handy internal MACRO used to increment over all the cores which are
 * valid AND enabled. This should be used STRICTLY as a FOR loop replacement!
 */
#define DVP_FOR_EACH_CORE(c)    for (c = dvp_rpc_next_core(DVP_CORE_MIN); c < DVP_CORE_REMOTE_MAX; c = dvp_rpc_next_core(c))

//******************************************************************************
// GLOBAL FUNCTIONS
//******************************************************************************

void dvp_rpc_mem_remove(DVP_RPC_t *rpc, DVP_PTR localAddr, DVP_U32 localSize, DVP_MemType_e memType)
{
    DVP_PTR remoteAddr = NULL;
    DVP_Core_e core;
    mutex_lock(&rpc->lock);
    if (memType == DVP_MTYPE_MPUCACHED_VIRTUAL)
    {
#if defined(DVP_USE_TILER)
        DVP_PTR tilerAddr = NULL;

        if (fph_get(rpc->mpu2tiler_hash, localAddr, &tilerAddr))
        {
            DVP_FOR_EACH_CORE(core)
            {
                if (fph_get(rpc->tiler2rpu_hash[core], tilerAddr, &remoteAddr)) {
#if defined(__QNX__)
                    ProcMgr_AddrInfo info;
                    memset(&info, 0, sizeof(info));
                    info.addr[ProcMgr_AddrType_SlaveVirt] = (UInt32)remoteAddr;
                    info.isCached = TRUE;
                    info.size = localSize;
                    ProcMgr_unmap(rpc->procs[core].handle, ProcMgr_MapType_Tiler, &info, ProcMgr_AddrType_SlaveVirt);
#else
                    ProcMgr_unmap(rpc->procs[core].handle, (UInt32)remoteAddr, dvp_rpc_getid(core));
#endif
                }

                fph_clr(rpc->tiler2rpu_hash[core], tilerAddr);
                fph_clr(rpc->rpu2tiler_hash[core], remoteAddr);
            }
            MemMgr_UnMap(tilerAddr);
            fph_clr(rpc->tiler2mpu_hash, tilerAddr);
            fph_clr(rpc->mpu2tiler_hash, localAddr);
        }
#endif
    }
    else
    {
        DVP_FOR_EACH_CORE(core)
        {
            // get the remote address
            if (fph_get(rpc->mpu2rpu_hash[core], localAddr, &remoteAddr))
            {
                int status = 0;
                DVP_PRINT(DVP_ZONE_MEM, "DVP RPC MEM: Found %p as %p on core %d, removing!\n", localAddr, remoteAddr, core);
#if !defined(__QNX__)
                // unmap that address
                if (localSize != 0)
                    status = ProcMgr_unmap(rpc->procs[core].handle, (UInt32)remoteAddr, dvp_rpc_getid(core));
#else
                if (localSize != 0) {
                    ProcMgr_AddrInfo info;
                    memset(&info, 0, sizeof(info));
                    info.addr[ProcMgr_AddrType_SlaveVirt] = (UInt32)remoteAddr;
                    info.isCached = TRUE;
                    info.size = localSize;
                    status = ProcMgr_unmap(rpc->procs[core].handle, ProcMgr_MapType_Tiler, &info, ProcMgr_AddrType_SlaveVirt);
                }
#endif
                DVP_PRINT(DVP_ZONE_MEM, "Unmap returned %d\n", status);
                // remove the local->remote hash
                fph_clr(rpc->mpu2rpu_hash[core], localAddr);
                // remove the remote->local hash
                fph_clr(rpc->rpu2mpu_hash[core], remoteAddr);

                DVP_PRINT(DVP_ZONE_MEM, "DVP RPC MEM: Removed %p from all hashes and cores!\n", localAddr);
            }
        }
    }
    mutex_unlock(&rpc->lock);
}

DVP_PTR dvp_rpc_mem_xlate_fwrd(DVP_RPC_t *rpc, DVP_Core_e core, DVP_PTR localAddr, DVP_U32 localSize, DVP_MemType_e memType)
{
    DVP_PTR remoteAddr = NULL;
    DVP_BOOL unwind = DVP_FALSE;
    ProcMgr_MapType maptype = 0;

    mutex_lock(&rpc->lock);
    switch (memType)
    {
#if defined(DVP_USE_TILER)
        case DVP_MTYPE_MPUCACHED_1DTILED:
        case DVP_MTYPE_MPUNONCACHED_1DTILED:
        case DVP_MTYPE_MPUNONCACHED_2DTILED:
        case DVP_MTYPE_DISPLAY_2DTILED:
            maptype = ProcMgr_MapType_Tiler;
            break;
#endif
        case DVP_MTYPE_MPUCACHED_VIRTUAL:
        default:
#if defined(DVP_USE_TILER)
#if defined(__QNX__)
            maptype = ProcMgr_MapType_Tiler;
            memType = DVP_MTYPE_MPUCACHED_1DTILED;
#else
            maptype = ProcMgr_MapType_Virt;
#endif
#endif
            break;
    }

    if (dvp_rpc_core_is_enabled(core))
    {
#if defined(DVP_USE_TILER)
        if (memType == DVP_MTYPE_MPUCACHED_1DTILED)
        {
            DVP_PTR tilerAddr = NULL;
            //DVP_PRINT(DVP_ZONE_MEM, "1D TILER CACHED XLate!\n");
            if (fph_get(rpc->mpu2tiler_hash, localAddr, &tilerAddr) == 0)
            {
                // not mapped already
                DVP_Core_e newCore;
                MemAllocBlock block;
                memset(&block, 0, sizeof(block));

                DVP_PRINT(DVP_ZONE_MEM, "1D TILER CACHED Mapping PTR:%p SIZE:%u\n",localAddr,localSize);

                // map to TILER, remember address
                block.ptr = localAddr;
                block.dim.len = localSize + (DVP_PAGE_SIZE - (localSize % DVP_PAGE_SIZE));
                block.stride = 0;
                block.pixelFormat = PIXEL_FMT_PAGE; /// @todo Revert this to the proper format as in the dvp_mem.c
                DVP_PRINT(DVP_ZONE_MEM, "1D TILER PTR:%p LEN:%u STR:%u FMT:%d\n",
                          block.ptr, block.dim.len,
                          block.stride, block.pixelFormat);
                tilerAddr = MemMgr_Map(&block, 1);
                if (tilerAddr != NULL)
                {
                    DVP_PRINT(DVP_ZONE_MEM, "Mapped to TILER = {p=%p,l=%u,s=%u,f=%u}\n",block.ptr, block.dim.len,block.stride,block.pixelFormat);
                    DVP_PRINT(DVP_ZONE_MEM, "LVA %p => TVA %p (SSPTR %p)\n",localAddr, tilerAddr, (DVP_PTR)block.reserved);
                    fph_set(rpc->mpu2tiler_hash, localAddr, tilerAddr);
                    // @TOOD Unwind mechanics for TILER mapping
                    fph_set(rpc->tiler2mpu_hash, tilerAddr, localAddr);
                    // map the TILER addr to each RPU, remember addresses
                    DVP_FOR_EACH_CORE(newCore)
                    {
                        DVP_S32 status = 0;
                        DVP_U32 addr;

                        SyslinkMemUtils_MpuAddrToMap mpuVirtAddr;

                        mpuVirtAddr.mpuAddr = (UInt32)tilerAddr;
                        mpuVirtAddr.size    = localSize;

                        status = SysLinkMemUtils_map(&mpuVirtAddr,
                                                     1,
                                                     &addr,
                                                     maptype,
                                                     newCore);
                        if (status >= 0)
                        {
                            DVP_PRINT(DVP_ZONE_MEM, "TVA %p => RVA[%u] %p\n",tilerAddr, newCore, (DVP_PTR)addr);
                            // remember the remote address
                            if (fph_set(rpc->tiler2rpu_hash[newCore], tilerAddr, (ptr_t)addr))
                            {
                                if (core == newCore)
                                    remoteAddr = (DVP_PTR)addr;

                                if (fph_set(rpc->rpu2tiler_hash[newCore], (ptr_t)addr, tilerAddr) == 0)
                                {
                                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to remember RVA %p to TVA %p in Hash!\n", remoteAddr, tilerAddr);
                                    unwind = DVP_TRUE;
                                }
                            }
                            else
                            {
                                DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to remember TVA %p to RVA %p in Hash!\n", tilerAddr, remoteAddr);
                                unwind = DVP_TRUE;
                            }
                        }
                        else
                        {
                            DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to map LVA %p => TVA %p to RVA!\n", localAddr, tilerAddr);
                            unwind = DVP_TRUE;
                        }
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to map 1D CACHED %p to TILER!\n", localAddr);
                    unwind = DVP_TRUE;
                }

            }
            else
            {
                DVP_PRINT(DVP_ZONE_RPC, "Found LVA:%p => TVA:%p in hash!\n", localAddr, tilerAddr);

                // found it in the MPU->TILER so we've mapped it already.
                if (DVP_VALID_REMOTE_CORE(core) && dvp_rpc_core_is_enabled(core))
                {
                    // now look it up in the TILER->RPU and return that
                    if (fph_get(rpc->tiler2rpu_hash[core], tilerAddr,  &remoteAddr) == 0)
                    {
                        DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to find TVA: %p in TILER to REMOTE CORE:%d hash!\n", tilerAddr, core);
                    }
                    else
                    {
                        DVP_PRINT(DVP_ZONE_RPC, "Found TVA:%p => RVA:%p in hash!\n", tilerAddr, remoteAddr);
                    }
                }

                if (core == DVP_CORE_CPU)
                {
                    // the tiler addr is the remote addr`
                    remoteAddr = tilerAddr;
                }
            }
        }
        else // 1d,2d uncached, virtual
        {
            DVP_Core_e mapCore = core;
            if (core == DVP_CORE_CPU)
            {
                // we're using the DVP_CORE_CPU as a way to prime the hashes with
                // pre-translated values... change to the first enabled remote
                // core
                mapCore = dvp_rpc_next_core(DVP_CORE_MIN);

                // there will be no translation, so just return the local pointer.
                remoteAddr = localAddr;
            }

            if (DVP_VALID_REMOTE_CORE(mapCore) && dvp_rpc_core_is_enabled(mapCore))
            {
                if (fph_get(rpc->mpu2rpu_hash[mapCore], localAddr, &remoteAddr) == 0)
                {
                    DVP_U32 addr;
                    DVP_Core_e newCore;
                    DVP_FOR_EACH_CORE(newCore)
                    {
                        DVP_S32 status = 0;
                        addr = 0;
                        SyslinkMemUtils_MpuAddrToMap mpuVirtAddr;

                        DVP_PRINT(DVP_ZONE_MEM, "Mapping PTR:%p SIZE:%d CORE:%d\n", localAddr, localSize, newCore);

                        mpuVirtAddr.mpuAddr = (UInt32)localAddr;
                        mpuVirtAddr.size    = localSize;

                        status = SysLinkMemUtils_map(&mpuVirtAddr,
                                                     1,
                                                     &addr,
                                                     maptype,
                                                     newCore);
                        if (addr != 0)
                        {
                            // install the pointer in the forward hash
                            if (fph_set(rpc->mpu2rpu_hash[newCore], localAddr, (ptr_t)addr))
                            {
                                // success! now remember the address if this is core
                                // we're interested in
                                if (core == newCore)
                                    remoteAddr = (DVP_PTR)addr;

                                // install the pointer in the backward hash
                                if (fph_set(rpc->rpu2mpu_hash[newCore], (ptr_t)addr, localAddr))
                                {
                                    // success! the pointer is in both hashes.
                                    DVP_PRINT(DVP_ZONE_MEM, "LVA %p => RVA[%u] %p\n", localAddr, newCore, (DVP_PTR)addr);
                                }
                                else // failed to remeber the backward translation
                                {
                                    DVP_PRINT(DVP_ZONE_WARNING, "WARNING! Address %p was not remembered in the Backward Fast Pointer Hash\n", (DVP_PTR)addr);
                                    // we must unwind previous mapping
                                    unwind = DVP_TRUE;
                                }
                            }
                            else // failed to remember the forward translation
                            {
                                DVP_PRINT(DVP_ZONE_WARNING, "WARNING! Address %p was not remembered in the Foward Fast Pointer Hash!\n", localAddr);
                                // we must unwind previous mapping
                                unwind = DVP_TRUE;
                            }
                        }
                        else // failed to map
                        {
                            DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to map local address %p to core %u (id:%d) (Status=%08x)\n", localAddr, newCore, dvp_rpc_getid(mapCore), status);
                            // we must unwind previous mapping
                            unwind = DVP_TRUE;
                        }
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_MEM, "Found LVA:%p => RVA:%p in hash!\n", localAddr, remoteAddr);
                }
            }
            else
            {
                DVP_PRINT(DVP_ZONE_WARNING, "No remote cores are enabled. No hash priming was done. Not an error.\n");
            }
        }
#endif // TILER enabled
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: INVALID CORE %d\n", core);
    }
    if (unwind == DVP_TRUE)
    {
        dvp_rpc_mem_remove(rpc, localAddr, localSize, memType);
        remoteAddr = NULL;
    }
    mutex_unlock(&rpc->lock);
    return remoteAddr;
}

DVP_PTR dvp_rpc_mem_xlate_back(DVP_RPC_t *rpc, DVP_Core_e core, DVP_PTR remoteAddr, DVP_MemType_e memType)
{
    DVP_PTR localAddr = NULL;
    mutex_lock(&rpc->lock);
    if (DVP_VALID_REMOTE_CORE(core) && dvp_rpc_core_is_enabled(core))
    {
#if defined(DVP_USE_TILER)
#if defined(__QNX__)
        if(memType == DVP_MTYPE_MPUCACHED_VIRTUAL)
            memType = DVP_MTYPE_MPUCACHED_1DTILED;
#endif
        if (memType == DVP_MTYPE_MPUCACHED_1DTILED)
        {
            DVP_PTR tilerAddr;
            fph_get(rpc->rpu2tiler_hash[core], remoteAddr, &tilerAddr);
            DVP_PRINT(DVP_ZONE_MEM, "TVA %p <= RVA[%u] %p\n", tilerAddr, core, remoteAddr);
            fph_get(rpc->tiler2mpu_hash, tilerAddr, &localAddr);
            DVP_PRINT(DVP_ZONE_MEM, "LVA %p <= TVA %p\n", localAddr, tilerAddr);
        }
        else
#endif
        {
            fph_get(rpc->rpu2mpu_hash[core], remoteAddr, &localAddr);
            DVP_PRINT(DVP_ZONE_MEM, "LVA %p <= RVA[%u] %p\n", localAddr, core, remoteAddr);
        }
    }
    mutex_unlock(&rpc->lock);
    return localAddr;
}


DVP_BOOL dvp_rpc_flush(DVP_RPC_t *rpc, DVP_Core_e core, DVP_PTR address, DVP_U32 size, DVP_MemType_e mtype)
{
    DVP_BOOL ret = DVP_FALSE;
    if (rpc)
    {
        DVP_PRINT(DVP_ZONE_RPC, "Flushing address:%p for %u bytes type:%d core:%d\n", address, size, mtype, core);
        if ((DVP_VALID_REMOTE_CORE(core) && dvp_rpc_core_is_enabled(core)) || core == DVP_CORE_CPU)
        {
            Int status = 0;
            DVP_PRINT(DVP_ZONE_RPC, "FLUSH CORE:%d PTR:%p SIZE:%u\n", core, address, size);
            status = ProcMgr_flushMemory(address, size, dvp_rpc_getid(core));
            if (status < 0) {
                DVP_PRINT(DVP_ZONE_ERROR, "DVP RPC: Failed to flush address %p on core %d\n", address, core);
            }
            else
                ret = DVP_TRUE;
        }
    }
    return ret;
}

DVP_BOOL dvp_rpc_invalidate(DVP_RPC_t *rpc, DVP_Core_e core, DVP_PTR address, DVP_U32 size, DVP_MemType_e mtype)
{
    DVP_BOOL ret = DVP_FALSE;
    if (rpc)
    {
        DVP_PRINT(DVP_ZONE_RPC, "Invalidating address:%p for %u bytes type:%d core:%d\n", address, size, mtype, core);
        if ((DVP_VALID_REMOTE_CORE(core) && dvp_rpc_core_is_enabled(core)) || core == DVP_CORE_CPU)
        {
            Int status = 0;
            DVP_PRINT(DVP_ZONE_RPC, "INVALIDATE CORE:%d PTR:%p SIZE:%u\n", core, address, size);
            status = ProcMgr_invalidateMemory(address, size, dvp_rpc_getid(core));
            if (status < 0) {
                DVP_PRINT(DVP_ZONE_ERROR, "DVP RPC: Failed to invalidate address %p on core %d\n", address, core);
            }
            else
                ret = DVP_TRUE;
        }
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

#if defined(DVP_USE_TILER)
    if (pImage->memType == DVP_MTYPE_DISPLAY_2DTILED)
        cached = DVP_FALSE;
#endif
#if defined(DVP_USE_TILER)
    if (pImage->memType == DVP_MTYPE_MPUNONCACHED_1DTILED ||
        pImage->memType == DVP_MTYPE_MPUNONCACHED_2DTILED)
        cached = DVP_FALSE;
#endif

    for (p = 0; p < pImage->planes; p++)
    {
        DVP_U32 planeSize = DVP_Image_PlaneSize(pImage, p);
        if (cached && cacheOp && !pImage->skipCacheOpFlush) {
            DVP_COMPLAIN_IF_FALSE(dvp_rpc_flush(rpc, core, pImage->pBuffer[p], planeSize, pImage->memType));
        }
        // @NOTE could the output buffers be accidently written back from the cache during operations?
        pImage->pBuffer[p] = dvp_rpc_mem_xlate_fwrd(rpc, core, pImage->pBuffer[p], planeSize, pImage->memType);
        // recalculate the new pData based on the starts and strides
        pImage->pData[p] = DVP_Image_Addressing(pImage, pImage->x_start, pImage->y_start, p);
     }
}

void dvp_rpc_return_image(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Image_t *pImage, DVP_BOOL cacheOp)
{
    DVP_BOOL cached = DVP_TRUE;
    DVP_U32 p = 0;

 #if defined(DVP_USE_TILER)
    if (pImage->memType == DVP_MTYPE_DISPLAY_2DTILED)
        cached = DVP_FALSE;
#endif
#if defined(DVP_USE_TILER)
    if (pImage->memType == DVP_MTYPE_MPUNONCACHED_1DTILED ||
        pImage->memType == DVP_MTYPE_MPUNONCACHED_2DTILED)
        cached = DVP_FALSE;
#endif
    for (p = 0; p < pImage->planes; p++)
    {
        DVP_U32 planeSize = DVP_Image_PlaneSize(pImage, p);
        pImage->pBuffer[p] = dvp_rpc_mem_xlate_back(rpc, core, pImage->pBuffer[p], pImage->memType);
        if (cached && cacheOp && !pImage->skipCacheOpInval) {
            DVP_COMPLAIN_IF_FALSE(dvp_rpc_invalidate(rpc, core, (DVP_PTR)pImage->pBuffer[p], planeSize, pImage->memType));
        }
        // recalculate the pData pointer
        pImage->pData[p] = DVP_Image_Addressing(pImage, pImage->x_start, pImage->y_start, p);
    }
}

void dvp_rpc_prepare_buffer(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Buffer_t *pBuffer, DVP_BOOL cacheOp, DVP_PTR base, DVP_RPC_Translation_t *trans)
{
    DVP_BOOL cached = DVP_TRUE;

    // Remove warnings
    base = base;
    trans = trans;

#if defined(DVP_USE_TILER)
    if (pBuffer->memType == DVP_MTYPE_MPUNONCACHED_1DTILED)
        cached = DVP_FALSE;
#endif
    if (cached && cacheOp && !pBuffer->skipCacheOpFlush) {
        DVP_COMPLAIN_IF_FALSE(dvp_rpc_flush(rpc, core, pBuffer->pData, pBuffer->numBytes, pBuffer->memType));
    }
    // @NOTE could the output buffers be accidently written back from the cache during operations?
    pBuffer->pData = dvp_rpc_mem_xlate_fwrd(rpc, core, pBuffer->pData, pBuffer->numBytes, pBuffer->memType);
}

void dvp_rpc_return_buffer(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Buffer_t *pBuffer, DVP_BOOL cacheOp)
{
    DVP_BOOL cached = DVP_TRUE;
#if defined(DVP_USE_TILER)
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
    return 0;
}

DVP_BOOL dvp_rpc_dissociate(DVP_RPC_t *rpc, DVP_Mem_t *mem, DVP_PTR ptr, DVP_VALUE *pHandle, size_t size, DVP_MemType_e mtype)
{
    DVP_BOOL ret = DVP_TRUE;

    if (rpc == NULL || mem == NULL || ptr == NULL || size == 0)
        return DVP_FALSE;

    switch (mtype)
    {
#if defined(DVP_USE_TILER)
        case DVP_MTYPE_MPUCACHED_1DTILED:
        case DVP_MTYPE_MPUNONCACHED_1DTILED:
        case DVP_MTYPE_MPUNONCACHED_2DTILED:
        case DVP_MTYPE_DISPLAY_2DTILED:
#endif
        case DVP_MTYPE_MPUCACHED_VIRTUAL:
        default:
            dvp_rpc_mem_remove(rpc, ptr, size, mtype);
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
#if defined(DVP_USE_TILER)
        case DVP_MTYPE_MPUCACHED_1DTILED:
        case DVP_MTYPE_MPUNONCACHED_1DTILED:
        case DVP_MTYPE_MPUNONCACHED_2DTILED:
        case DVP_MTYPE_DISPLAY_2DTILED:
#endif
        case DVP_MTYPE_MPUCACHED_VIRTUAL:
        default:
            dvp_rpc_mem_xlate_fwrd(rpc, DVP_CORE_CPU, ptr, size, mtype);
            break;
    }
    return ret;
}

void dvp_rpc_deinit(DVP_RPC_t **rpc)
{
    if (rpc && *rpc)
    {
        Int status;
        DVP_Core_e core = 0;
        DVP_FOR_EACH_CORE(core)
        {
            fph_deinit((*rpc)->mpu2rpu_hash[core]);
            fph_deinit((*rpc)->rpu2mpu_hash[core]);
#if defined(DVP_USE_TILER)
            fph_deinit((*rpc)->tiler2rpu_hash[core]);
            fph_deinit((*rpc)->rpu2tiler_hash[core]);
#endif
            RCM_COMPLAIN_IF_FAILED(status, ProcMgr_detach((*rpc)->procs[core].handle));
            RCM_COMPLAIN_IF_FAILED(status, ProcMgr_close(&(*rpc)->procs[core].handle));
        }
#if defined(DVP_USE_TILER)
        fph_deinit((*rpc)->tiler2mpu_hash);
        fph_deinit((*rpc)->mpu2tiler_hash);
#endif
        RCM_COMPLAIN_IF_FAILED(status, Ipc_destroy());
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
        DVP_Core_e core = 0;
        Int status = 0;
        Ipc_getConfig(&rpc->config);
        status = Ipc_setup(&rpc->config);
        if (status < 0)
        {
            uint32_t c;
            DVP_PRINT(DVP_ZONE_ERROR, "Failed to initialize IPC mechanism, no RPC possible\n");
            // disable all cores
            for (c = 0; c < dimof(dvp_core_names); c++)
                dvp_core_names[c].enabled = false_e;
            free(rpc);
            rpc = NULL;
        }
        else
        {
            mutex_init(&rpc->lock);
#if defined(DVP_USE_TILER)
            rpc->mpu2tiler_hash = fph_init(DVP_RPC_NUM_BLOCKS, DVP_RPC_BLOCK_SIZE, NULL);
            rpc->tiler2mpu_hash = fph_init(DVP_RPC_NUM_BLOCKS, DVP_RPC_BLOCK_SIZE, NULL);
#endif
            DVP_FOR_EACH_CORE(core)
            {
                DVP_PRINT(DVP_ZONE_RPC, "Core [%u] = %s\n", core, rpc->config.multiProcConfig.nameList[core]);
                rpc->procs[core].id = MultiProc_getId(dvp_core_names[core].name);
                status = ProcMgr_open(&rpc->procs[core].handle, rpc->procs[core].id);
                if (status > 0)
                {
                    ProcMgr_getAttachParams(rpc->procs[core].handle, &rpc->procs[core].params);
                    status = ProcMgr_attach(rpc->procs[core].handle, &rpc->procs[core].params);
                    if (status < 0)
                    {
                        DVP_PRINT(DVP_ZONE_ERROR, "Failed to attach to Core[%u] %s, disabling\n", core, rpc->config.multiProcConfig.nameList[core]);
                        dvp_core_names[core].enabled = false_e;
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "Failed to open Core[%u] %s, disabling\n", core, rpc->config.multiProcConfig.nameList[core]);
                    dvp_core_names[core].enabled = false_e;
                }
                if (dvp_core_names[core].enabled == true_e)
                {
                    // we only allocate hashes for cores which are enabled.
                    rpc->mpu2rpu_hash[core] = fph_init(DVP_RPC_NUM_BLOCKS, DVP_RPC_BLOCK_SIZE, NULL);
                    rpc->rpu2mpu_hash[core] = fph_init(DVP_RPC_NUM_BLOCKS, DVP_RPC_BLOCK_SIZE, NULL);
#if defined(DVP_USE_TILER)
                    rpc->tiler2rpu_hash[core] = fph_init(DVP_RPC_NUM_BLOCKS, DVP_RPC_BLOCK_SIZE, NULL);
                    rpc->rpu2tiler_hash[core] = fph_init(DVP_RPC_NUM_BLOCKS, DVP_RPC_BLOCK_SIZE, NULL);
#endif
                }
           } // for each core
        }
    }
    DVP_Perf_Clear(&rpc_perf);
    return rpc;
}

void dvp_rpc_core_deinit(DVP_RPC_t *rpc, DVP_RPC_Core_t **rpcc)
{
    if (rpc && rpcc && *rpcc)
    {
        DVP_Core_e core = (*rpcc)->coreEnum;
        if (DVP_VALID_REMOTE_CORE(core) && dvp_rpc_core_is_enabled(core))
        {
            dvp_core_names[core].enabled = false_e;
            dvp_rpc_client_deinit(&(*rpcc)->client);
            dvp_rpc_server_deinit(&(*rpcc)->server);
        }
        free(*rpcc);
        *rpcc = NULL;
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
            // register the RcmServer
            strncpy(rpcc->server.name, dvp_core_names[core].server_name, DVP_MAX_SERVER_NAME);
            rpcc->server.calls = rpci->localFunctions;
            rpcc->server.numCalls = rpci->numLocalFunctions;
            RCM_GOTO_IF_FAILED(status, dvp_rpc_server_init(&rpcc->server), leave);

            // register the RcmClient
            strncpy(rpcc->client.name, dvp_core_names[core].client_name, DVP_MAX_SERVER_NAME);
            rpcc->client.calls = rpci->remoteFunctions;
            rpcc->client.numCalls = rpci->numRemoteFunctions;
            RCM_GOTO_IF_FAILED(status, dvp_rpc_client_init(&rpcc->client), leave);
            if (status < 0)
                goto leave;
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

    RcmClient_Message *pPacket = NULL;
    RcmClient_Message *pResponse = NULL;
    Int status = 0;
    DVP_U32 msgSize = 0;
    UInt8 *pMsg = NULL;
    DVP_U32 i = 0;

    // calculate the message size
    for (i = 0; i < numParams; i++)
        msgSize += params[i].size;

    DVP_PRINT(DVP_ZONE_RPC, "RPC: Sending %u pararms, %u bytes\n", numParams, msgSize);

    // allocate the packet
    status = RcmClient_alloc(rpcc->client.handle, msgSize, &pPacket);
    if (status >= 0)
    {
        // set the packet values
        pPacket->fxnIdx = rpcc->client.calls[cliIndex].index;

        // assign the msg pointer
        pMsg = (UInt8 *)&pPacket->data[0];

        // marshall the parameters
        for (i = 0; i < numParams; i++)
        {
            if (params[i].type == DVP_RPC_READ_ONLY ||
                params[i].type == DVP_RPC_READ_AND_WRITE)
            {
                memcpy(pMsg, params[i].data, params[i].size);
                pMsg += params[i].size;
            }
        }

        DVP_PRINT(DVP_ZONE_RPC, "DVP_RPC: Calling %s::%s[%u] with %u params\n", rpcc->client.name, rpcc->client.calls[cliIndex].name, rpcc->client.calls[cliIndex].index, numParams);
        for (i = 0; i < numParams; i++)
        {
            DVP_PRINT(DVP_ZONE_RPC, "DVP_RPC:\t[%u] TYPE:%04x SIZE:%u PTR:%p DATA:0x%x\n", i, params[i].type, params[i].size, params[i].data, *(DVP_U32 *)params[i].data);
        }

#if (DVP_DEBUG == 1)
        // flush out the PRINTF's before we may crash the system
        fflush(stdout);
#endif

        // call the remote function!
        status = RcmClient_exec(rpcc->client.handle, pPacket, &pResponse);
        DVP_PRINT(DVP_ZONE_RPC, "RcmClient_exec returned 0x%08x (result = 0x%08x)\n", status, pPacket->result);
        if (status >= 0)
        {
            // if successful, reset the pMsg to the response packet
            pMsg = (UInt8 *)&pResponse->data[0];

            for (i = 0; i < numParams; i++)
            {
                if (params[i].type == DVP_RPC_WRITE_ONLY ||
                    params[i].type == DVP_RPC_READ_AND_WRITE)
                {
                    DVP_PRINT(DVP_ZONE_RPC, "Write back %p => %p for %u bytes\n", pMsg, params[i].data, params[i].size);
                    memcpy(params[i].data, pMsg, params[i].size);
                    switch (params[i].size)
                    {
                        case 1:
                        {
                            UInt8 *pData = (UInt8 *)params[i].data;
                            DVP_PRINT(DVP_ZONE_RPC, "\t[%u] = 0x%02x\n", i, *pData);
                            break;
                        }
                        case 2:
                        {
                            UInt16 *pData = (UInt16 *)params[i].data;
                            DVP_PRINT(DVP_ZONE_RPC, "\t[%u] = 0x%04x\n", i, *pData);
                            break;
                        }
                        case 4:
                        {
                            DVP_U32 *pData = (DVP_U32 *)params[i].data;
                            DVP_PRINT(DVP_ZONE_RPC, "\t[%u] = 0x%08x\n", i, *pData);
                            break;
                        }
                        default:
                        {
                            UInt8 *pData = (UInt8 *)params[i].data;
                            DVP_U32 j = params[i].size;
                            DVP_PRINT(DVP_ZONE_RPC, "\t[%u] = ",i);
                            for (j = 0; j < params[i].size; j++) {
                                DVP_PRINT(DVP_ZONE_RPC, " 0x%02x", *pData);
                            }
                            DVP_PRINT(DVP_ZONE_RPC, "\n");
                            break;
                        }
                    }
                    pMsg += params[i].size;
                }
            }
            // remember the remote result!
            status = pPacket->result;
            RcmClient_free(rpcc->client.handle, pResponse);
            pResponse = NULL;

            DVP_PRINT(DVP_ZONE_RPC, "Remote Call Successful! (Ret=0x%08x:%d)\n", status, status);
        }
        else
        {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR: RPC Failed to call remote function!\n");
            RcmClient_free(rpcc->client.handle, pPacket);
        }
        pPacket = NULL;
    }
    return status;
}

#endif

//******************************************************************************
// END
//******************************************************************************
