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

#if defined(DVP_USE_BIOSIPC)

//******************************************************************************
// INCLUDES
//******************************************************************************

#include <sosal/sosal.h>

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>
// @NOTE we include dvp_rpc.h further down in the file...

#include <Std.h>
#include <IpcUsr.h>

//******************************************************************************
// #DEFINES
//******************************************************************************

#define DVP_RPC_BLOCK_SIZE  (64)
#define DVP_RPC_NUM_BLOCKS  (64)

#define PROC_VALID_ID(x)     (-1 < (x) && (x) < PROC_END)

//******************************************************************************
// TYPEDEFS
//******************************************************************************

typedef struct _dvp_rpc_t {
    DVP_Handle handle;         /** This contains the pointer back the DVP_t. It MUST be the FIRST entry in the structure! */
    mutex_t    lock;
} DVP_RPC_t;

#define DVP_RPC_T   // we're defining the RPC structure locally!
#include <dvp_rpc.h>

#define DVP_VALID_REMOTE_CORE(x)    (DVP_CORE_MIN < (x) && (x) < DVP_CORE_REMOTE_MAX)


//******************************************************************************
// LOCAL VARIABLES
//******************************************************************************


//******************************************************************************
// LOCAL FUNCTIONS
//******************************************************************************

DVP_RPC_t *dvp_rpc_init()
{
    // attach to all the cores...
    // create a message q heap
    // create the messageq
}

void dvp_rpc_deinit(DVP_RPC_t **rpc) {}

void dvp_rpc_restart_callback_register(DVP_RPC_t *rpc, DVP_RPC_Core_t *rpcc, dvp_rpc_restart_f func, void *cookie) {}

DVP_PTR dvp_rpc_mem_xlate_fwrd(DVP_RPC_t *rpc, DVP_Core_e core, DVP_PTR localAddr, DVP_U32 localSize, DVP_MemType_e memType)
{
    return NULL;
}

DVP_PTR dvp_rpc_mem_xlate_back(DVP_RPC_t *rpc, DVP_Core_e core, DVP_PTR remoteAddr, DVP_MemType_e memType)
{
    return NULL;
}

void dvp_rpc_mem_remove(DVP_RPC_t *rpc, DVP_PTR localAddr, DVP_U32 localSize, DVP_MemType_e memType)
{
    ;
}

DVP_RPC_Core_t *dvp_rpc_core_init(DVP_RPC_t *rpc, DVP_RPC_Interface_t *rpci)
{
    // connect to each remote core
}

void dvp_rpc_core_deinit(DVP_RPC_t *rpc, DVP_RPC_Core_t **rpcc)
{
    // disconnect from each core
}

DVP_BOOL dvp_rpc_dissociate(DVP_RPC_t *rpc, DVP_Mem_t *mem, DVP_PTR ptr, DVP_VALUE *pHandle, size_t size, DVP_MemType_e mtype)
{
    return DVP_FALSE;
}

DVP_BOOL dvp_rpc_associate(DVP_RPC_t *rpc,  DVP_Mem_t *mem, DVP_PTR ptr, DVP_VALUE handle, size_t size, DVP_MemType_e mtype)
{
    return DVP_FALSE;
}

DVP_VALUE dvp_rpc_get_handle(DVP_RPC_t *rpc, DVP_PTR ptr)
{
    return 0;
}

DVP_S32 dvp_rpc_remote_execute(DVP_RPC_t *rpc,
                               DVP_RPC_Core_t *rpcc,
                               DVP_U32 cliIndex,
                               DVP_RPC_Parameter_t *params,
                               DVP_U32 numParams,
                               DVP_RPC_Translation_t *trans)
{
    return 0;
}

DVP_BOOL dvp_rpc_invalidate(DVP_RPC_t *rpc, DVP_Core_e core, DVP_PTR address, DVP_U32 size, DVP_MemType_e mtype)
{
    return DVP_FALSE;
}

DVP_BOOL dvp_rpc_flush(DVP_RPC_t *rpc, DVP_Core_e core, DVP_PTR address, DVP_U32 size, DVP_MemType_e mtype)
{
    return DVP_FALSE;
}

void dvp_rpc_prepare_image(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Image_t *pImage, DVP_BOOL cacheOp, DVP_PTR base, DVP_RPC_Translation_t *trans)
{
    ;
}

void dvp_rpc_return_image(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Image_t *pImage, DVP_BOOL cacheOp)
{
    ;
}

void dvp_rpc_prepare_buffer(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Buffer_t *pBuffer, DVP_BOOL cacheOp, DVP_PTR base, DVP_RPC_Translation_t *trans)
{
    ;
}

void dvp_rpc_return_buffer(DVP_RPC_t *rpc, DVP_Core_e core, DVP_Buffer_t *pBuffer, DVP_BOOL cacheOp)
{
    ;
}

#endif

//******************************************************************************
// END
//******************************************************************************

