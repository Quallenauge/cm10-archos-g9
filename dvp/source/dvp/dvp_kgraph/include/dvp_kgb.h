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
 * \brief The DVP Kernel Graph Boss.
 * \defgroup group_dvp_kgb DVP Kernel Graph Boss API.
 * \brief The API related the Kernel Graph Boss.
 */

#ifndef _DVP_KGB_H_
#define _DVP_KGB_H_

// external
#include <dvp/dvp_types.h>
#include <dvp/dvp_mem.h>

// internal
#include <dvp_kgm.h>
#include <dvp_mem_int.h>
#include <dvp_rpc.h>

#if defined(DVP_USE_ION)
#include <ion/ion.h>
#if defined(ANDROID)
#include <hwc/hal_public.h>
#endif
#endif

#ifdef DVP_USE_GRALLOC
#include <hwc/hal_public.h>
#endif

/*! \brief The magic number used to determine if the load table has been initialized.
 * \ingroup group_dvp_kgb
 */
#define DVP_LOAD_TABLE_NAME            (0xDEADFACE)

/*! \brief The flag to indicate the the KGMs should be initialized.
 * \ingroup group_dvp_kgb
 */
#define DVP_KGB_INIT_KGMS  (1 << 0)

/*! \brief The flags to indicate that the Memory system should be initialized.
 * \ingroup group_dvp_kgb
 */
#define DVP_KGB_INIT_MEM_MGR   (1 << 1)

/*! \brief The set of all flags.
 * \ingroup group_dvp_kgb
 */
#define DVP_KGB_INIT_ALL  (DVP_KGB_INIT_KGMS | DVP_KGB_INIT_MEM_MGR)

/*! \brief The core load structure.
 * \ingroup group_dvp_kgb
 */
typedef struct _dvp_core_load_t {
    DVP_S32     maximumLoad;
    DVP_S32     currentLoad;
} DVP_Core_Load_t;

/*! \brief The system load structure.
 * \ingroup group_dvp_kgb
 */
typedef struct _dvp_load_t {
    bool_e          initialized;
    semaphore_t     sem;
    DVP_Core_Load_t cores[DVP_CORE_MAX];
} DVP_Load_t;

/*! \brief The DVP Kernel Graph Lock.
 * \ingroup group_dvp_kgb
 */
typedef struct _dvp_graph_lock_t {
    bool_e              m_enabled;
    mutex_t             m_lock;
    event_t             m_wait;
    uint32_t            m_count;
} DVP_GraphLock_t;

/*! \brief The internal top level context structure for DVP.
 * \ingroup group_dvp_kgb
 */
typedef struct _dvp_t {
    DVP_GraphManager_t *managers;
    DVP_U32             numMgrs;
    mutex_t             mgrLock;
    shared_t           *shared_memory;
    DVP_Load_t         *loads;
    DVP_RPC_t          *rpc;
    DVP_Mem_t          *mem;
    DVP_GraphLock_t     graphLock;
} DVP_t;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief This function initializes the graph boss.
 * \return DVP_t *
 * \ingroup group_dvp_kgb
 */
DVP_t *DVP_KernelGraphBossInit();

/*!
 * \brief This function deinitializes the graph boss.
 * \param [in] dvp The handle to the dvp instance.
 * \ingroup group_dvp_kgb
 */
void DVP_KernelGraphBossDeinit(DVP_t *dvp);

/*!
 * \brief This function verifies a single section of the graph.
 * \param [in] dvp The pointer to the DVP_t context.
 * \param [in] section The pointer to the section to verify.
 * \return Returns the number of valid nodes.
 * \ingroup group_dvp_kgb
 */
DVP_U32 DVP_KernelGraphBoss_Verify(DVP_t *dvp,  DVP_KernelGraphSection_t *section);

/*!
 * \brief This function process a single section of the graph in either a synchronous or asychronous mode.
 * \param [in] dvp The pointer to the DVP_t context.
 * \param [in] section The pointer to the section to process.
 * \param [in] sync A boolean indicating if the section should be executed in-line with this call.
 * \ingroup group_dvp_kgb
 */
DVP_U32 DVP_KernelGraphBoss_Process(DVP_t *dvp,  DVP_KernelGraphSection_t *section, DVP_BOOL sync);

/*!
 * \brief This function allows the caller to limit max load of a requested core in the shared load table.
 * \param [in] dvp The pointer to the DVP_t context.
 * \param [in] core
 * \param [in] limit
 * \ingroup group_dvp_kgb
 */
void DVP_SetMaxLoad(DVP_t *dvp, DVP_Core_e core, DVP_U32 limit);

/**
 * This function allows the caller to retrieve the max load of the requested core from the shared load table.
 * \param [in] dvp The pointer to the DVP_t context.
 * \param [in] core     The core being queried.
 * \param [in] pMaxLoad The maximum load reported from the manager.
 * \ingroup group_dvp_kgb
 */
DVP_U32 DVP_GetMaxLoad(DVP_t *dvp, DVP_Core_e core, DVP_U32 *pMaxLoad);

/**
 * \brief Queries an individual core in DVP.
 * \param [in] dvp The pointer to the DVP_t context.
 * \param [in] core The core to query.
 * \param [in] info A pointer to a preallocated structure which will contain the core
 * info.
 * \ingroup group_dvp_kgb
 */
void DVP_QueryCore(DVP_t *dvp, DVP_Core_e core, DVP_CoreInfo_t *info);

/*!
 * \brief Queries the boss to determine if the specified core supports this kernel.
 * \param [in] dvp The pointer to the DVP_t context.
 * \param [in] kernel The kenel enum to search for.
 * \param [in] core The core to search.
 * \retval DVP_TRUE The core supports the kernel
 * \retval DVP_FALSE The core does not support the kernel.
 * \ingroup group_dvp_kgb
 */
DVP_BOOL DVP_QueryCoreForKernel(DVP_t *dvp, DVP_KernelNode_e kernel, DVP_Core_e core);

/*! This function initializes the memory system.
 * \ingroup group_dvp_kgb
 */
DVP_Mem_t *dvp_mem_init();

/*! This function disables the memory system.
 * \param [in,out] pmem The pointer to the memory structure, which will be set to NULL, after freeing.
 * \ingroup group_dvp_kgb
 */
void dvp_mem_deinit(DVP_Mem_t **pmem);

#ifdef __cplusplus
}
#endif


#endif
