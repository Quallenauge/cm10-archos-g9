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

#ifndef _DVP_KGM_H_
#define _DVP_KGM_H_

/*!
 * \file
 * \brief An internal header file which defines the DVP Kernel Graph Manager interface.
 * \author Erik Rainey <erik.rainey@ti.com>
 * \defgroup group_dvp_kgm DVP Kernel Graph Manager API
 */

#include <sosal/sosal.h>

// external
#include <dvp/dvp_types.h>
// internal
#include <dvp_rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


/*! \brief The function pointer to the KGM initialization function
 * \ingroup group_dvp_kgm
 */
typedef DVP_BOOL (*DVP_GraphManagerInit_f)(DVP_RPC_t *rpc, DVP_RPC_Core_t *pCore);

/*! The function pointer to the KGM deinitialization function
 * \ingroup group_dvp_kgm
 */
typedef DVP_BOOL (*DVP_GraphManagerDeinit_f)(void);

/*! \brief This function pointer is called when the remote endpoint unexpectedly restarts.
 * \ingroup group_dvp_kgm
 */
typedef void (*DVP_GraphManagerRestart_f)(void *arg);

/*! \brief The function pointer to the function which processes the subarray of kernel nodes.
 * \ingroup group_dvp_kgm
 */
typedef DVP_U32 (*DVP_GraphManager_f)(DVP_KernelNode_t *pSubNodes, DVP_U32 startNode, DVP_U32 numNodes, DVP_BOOL sync);

/*! \brief The function pointer to the function which returns the pointer the list of support kernels.
 * \ingroup group_dvp_kgm
 */
typedef DVP_U32 (*DVP_GetSupportedKernels_f)(DVP_CoreFunction_t **pFuncs);

/*! \brief The function pointer to the function which returns the list of remote functions that the local cores will call.
 * \ingroup group_dvp_kgm
 */
typedef DVP_U32 (*DVP_GetSupportedRemoteCalls_f)(DVP_Function_t **pRemote, DVP_U32 *pNum);

/*! \brief The function pointer to the function which returns the list of local functions that remote cores and call
 * \ingroup group_dvp_kgm
 */
typedef DVP_U32 (*DVP_GetSupportedLocalCalls_f)(DVP_Function_t **pLocal, DVP_U32 *pNum);

/*! \brief The function pointer to the function which returns the enumeration of the supported core that this object manages.
 * \ingroup group_dvp_kgm
 */
typedef DVP_Core_e (*DVP_GetSupportedRemoteCore_f)(void);

/*! \brief The function pointer to the function which returns the maximum load of the remote core
 * \ingroup group_dvp_kgm
 */
typedef DVP_U32 (*DVP_GetMaximumLoad_f)(void);

/*! \brief The function pointer to the function which verifies the graph before execution.
 * \ingroup group_dvp_kgm
 */
typedef  DVP_U32 (*DVP_KernelGraphManagerVerify_f)(DVP_KernelNode_t *pSubNodes, DVP_U32 startNode, DVP_U32 numNodes);

/*!
 * \brief This is the interface structure to a DVP Kernel Graph Manager.
 * \ingroup group_dvp_kgm
 */
typedef struct _dvp_graph_manager_funcs_t {
    DVP_GraphManagerInit_f        init;
    DVP_GraphManager_f            manager;
    DVP_GetSupportedKernels_f     getKernels;
    DVP_GetSupportedLocalCalls_f  getLocal;
    DVP_GetSupportedRemoteCalls_f getRemote;
    DVP_GetSupportedRemoteCore_f  getCore;
    DVP_GetMaximumLoad_f          getLoad;
    DVP_GraphManagerDeinit_f      deinit;
    DVP_GraphManagerRestart_f     restart;
    DVP_KernelGraphManagerVerify_f verify;
} DVP_GraphManager_Calls_t;

/*! \brief This indicates that the manager's priority is invalid and will not be used.
 * \ingroup group_dvp_kgm
 */
#define DVP_PRIORITY_NONE    (0)

/*! \brief This is used to move the priority from the higher to lower priority
 * \ingroup group_dvp_kgm
 */
#define DVP_PRIORITY_INC     (1)

/*! \brief This indicates the highest priority
 * \ingroup group_dvp_kgm
 */
#define DVP_PRIORITY_HIGHEST (1)

/*! \brief This indicates the base priority for user supplied managers.
 * \ingroup group_dvp_kgm
 */
#define DVP_PRIORITY_USER_BASE (1000)

/*! \brief This indicates the lowest priority.
 * \ingroup group_dvp_kgm
 */
#define DVP_PRIORITY_LOWEST  (0xFFFFFFFE)

/*! \brief This indicates the maximum value a priority field can logically have.
 * \ingroup group_dvp_kgm
 */
#define DVP_PRIORITY_MAX     (0xFFFFFFFF)

/*! \brief The DVP Kernel Graph Manager data structure.
 * \ingroup group_dvp_kgm
 */
typedef struct _dvp_graph_manager_t {
    /*! \brief This indicates whether the manager is enabled or not. */
    bool_e                      enabled;
    /*! \brief The short name of the kernel graph manager */
    char                        name[10];
    /*! \brief The file name of the loaded module */
    char                        dlname[MAX_PATH];
    /*! \brief The handle to the laoded module */
    module_t                    handle;
    /*! \brief Function interface to the KGM. */
    DVP_GraphManager_Calls_t    calls;
    /*! \brief The pointer to the list of supported kernels by this manager */
    DVP_CoreFunction_t         *kernels;
    /*! \brief The number of supported kernels by this manager */
    DVP_U32                     numSupportedKernels;
    /*! \brief The poitner to the remote procedure call core data structure */
    DVP_RPC_Core_t             *rpcc;
    /*! \brief The pointer to the remote procedure call interface */
    DVP_RPC_Interface_t         rpci;
    /*! \brief The priority of the KGM. 0 is invalid, 1 is Highest, 2 is next highest, etc. See DVP_PRIORITY_XXX macros. */
    DVP_U32                     priority;
} DVP_GraphManager_t;

#if defined(DVP_USE_OMAPRPC)
#include <omaprpc/omaprpc.h>
#define DVP_KGM_FUNC_BASE OMAPRPC_REMOTE_FUNC_BASE
#else
#define DVP_KGM_FUNC_BASE (0)
#endif

/*! \brief These are used as indexes into the Remote DVP KGM.
 * \ingroup group_dvp_kgm
 */
typedef enum _dvp_kgm_function_e {
    /*! \brief The index to the remote initialization function */
    DVP_KGM_REMOTE_INIT = DVP_KGM_FUNC_BASE,
    /*! \brief The index to the remote deinitialization function */
    DVP_KGM_REMOTE_DEINIT,
    /*! \brief The index to the remote execution function */
    DVP_KGM_REMOTE_EXEC,
} DVP_KGM_Remote_Function_e;

/*!< \brief This is used internally to implement a per-core thread model.
 * \ingroup group_dvp_kgm
 */
typedef struct _dvp_kgm_thread_t {
    DVP_KernelNode_t *pSubNodes;    /*!< \brief The pointer to the node array to execute */
    DVP_U32 startNode;              /*!< \brief The index of the node in DVP_KGM_Thread_t::pSubNodes to start execution from */
    DVP_U32 numNodes;               /*!< \brief The number of nodes to execute on DVP_KGM_Thread_t::pSubNodes starting at DVP_KGM_Thread_t::startNode */
    DVP_U32 numNodesExecuted;       /*!< \brief The number of nodes actually executed. */
} DVP_KGM_Thread_t;

#ifdef __cplusplus
}
#endif

#endif
