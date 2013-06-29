/*
 *  Copyright (C) 2009-2011 Texas Instruments, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use This file except in compliance with the License.
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
 * \brief The DVP RPC layer.
 * \defgroup group_dvp_rpc DVP Remote Procedure Calls API.
 */

#ifndef _DVP_RPC_H_
#define _DVP_RPC_H_

#include <dvp/dvp_types.h>
#include <dvp_mem_int.h>

/*! \brief Defines an warning logging mechanism for RPC calls.
 * \ingroup group_dvp_rpc
 */
#define RPC_COMPLAIN_IF_FAILED(ret, statement) { \
    ret = statement;\
    if (ret < 0) { \
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: "#statement" returned %d\n", ret);\
    }\
}

#ifndef DVP_RPC_T
/*! \brief This forward declaration is to prevent needless #includes
 * The DVP_RPC library has it's own internal definitions of these which pulls in
 * the definitions of foreign libraries which we're trying to not have
 * clients of this file have to include or add to their include paths.
 * \ingroup group_dvp_rpc
 */
typedef struct _dvp_rpc_t {
    DVP_Handle handle; /*!< back reference to top level structure. */
} DVP_RPC_t;
typedef DVP_U32 DVP_RPC_Core_t;
#endif

/*! \brief The maximum name length of a remote function
 * \ingroup group_dvp_rpc
 */
#define DVP_RPC_MAX_FUNC_NAME_LEN   (80)

/*! \brief A pointer type to a remote dvp function */
typedef unsigned int (*DVP_Function_f)(unsigned int, unsigned int *);

/*! \brief Defines a DVP function and it's attributes
 * \ingroup group_dvp_rpc
 */
typedef struct _dvp_function_t {
    /*! The name of the function */
    char           name[DVP_RPC_MAX_FUNC_NAME_LEN];
    /*! The pointer to the function */
    DVP_Function_f function;
    /*! The index to the function in a table */
    DVP_U32        index;
} DVP_Function_t;

/*! \brief Defines the DVP RPC interface for KGMs.
 * \ingroup group_dvp_rpc
 */
typedef struct _dvp_rpc_interface_t {
    /*! These are the calls into the library from the remote core */
    DVP_Function_t *localFunctions;
    /*! The number of local functions */
    DVP_U32         numLocalFunctions;
    /*! These are the calls from the library into the remote core */
    DVP_Function_t *remoteFunctions;
    /*! The number of remote functions */
    DVP_U32         numRemoteFunctions;
    /*! The core which the KGM interacts with */
    DVP_Core_e      coreEnum;
} DVP_RPC_Interface_t;

/*! \brief Defines the direction a parameter can go
 * \ingroup group_dvp_rpc
 */
typedef enum _dvp_rpc_parameter_e {
    DVP_RPC_READ_ONLY,          /*!< This indicates MPU to RPU only */
    DVP_RPC_WRITE_ONLY,         /*!< This indicates RPU to MPU only */
    DVP_RPC_READ_AND_WRITE,     /*!< This indicates both MPU to RPU and RPU
                                      back to MPU */
} DVP_RPC_Parameter_e;

/*! \brief Defines a pointer translation structure
 * \ingroup group_dvp_rpc
 */
typedef struct _dvp_rpc_ptr_translation_t {
    DVP_U32  index;
    DVP_U32  offset;
    DVP_U32  base;
    DVP_BOOL cacheops;
    DVP_PTR  reserved;
} DVP_RPC_PointerTranslation_t;

/*! \brief Defines a maximum number of pointer translations
 * \ingroup group_dvp_rpc
 */
#define DVP_MAX_TRANSLATIONS (1024)

/*! \brief Define the number of times DVP will retry a restart before giving up.
 * \ingroup group_dvp_rpc
 */
#define DVP_MAX_RESTART_RETRY (100)

/*! \brief This structure maintains the list of translations for a call.
 * \ingroup group_dvp_rpc
 */
typedef struct _dvp_rpc_translation_t {
    DVP_U32                      numTranslations;
    DVP_RPC_PointerTranslation_t translations[DVP_MAX_TRANSLATIONS];
} DVP_RPC_Translation_t;

/*! \brief This structure describes a parameter which will be passed over RPC.
 * \ingroup group_dvp_rpc
 */
typedef struct _dvp_rpc_parameter_t {
    DVP_RPC_Parameter_e  type;      /*!< The type or direction of the
                                         parameter. */
    DVP_U32              size;      /*!< The size of the data pointed to. */
    DVP_PTR              data;      /*!< The data pointer to send */
    DVP_PTR              reserved;  /*!< In ANDROID this should be used to
                                         hold the handle to the memory */
} DVP_RPC_Parameter_t;

/*! \brief The callback type for unexpected restarts on the remote endpoint.
 * \param [in] cookie The user private pointer.
 * \ingroup group_dvp_rpc
 */
typedef void (*dvp_rpc_restart_f)(void *cookie);

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief This function initializes the IPC/Syslink layers for all remote cores
 * \ingroup group_dvp_rpc
 */
DVP_RPC_t *dvp_rpc_init();

/*!
 * \brief This function deinitializes the IPC/Syslink layers for all remote
 * cores
 * \param [in] rpc The pointer to the pointer to the RPC context. This will be
 * set to NULL
 * during this function.
 * \ingroup group_dvp_rpc
 */
void dvp_rpc_deinit(DVP_RPC_t **rpc);

/*!
 * \brief This function registers a client's restart callback.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] rpcc The pointer to the RPC Core context.
 * \param [in] func The function to call after an unexpected restart of the
 * remote endpoint.
 * \param [in] cookie The user private data to pass to the function.
 */
void dvp_rpc_restart_callback_register(DVP_RPC_t *rpc,
                                       DVP_RPC_Core_t *rpcc,
                                       dvp_rpc_restart_f func,
                                       void *cookie);

/*!
 * \brief This function translates a local virtual address into a remote
 * virtual address. If the buffer has not been mapped it will map it unto all
 * the remote cores. If the buffer has been mapped before it will quickly
 * lookup the remote address and return it.
 * \param rpc The pointer to the RPC context.
 * \param core The remote core to get the virtual address for.
 * \param localAddr The local virtual address.
 * \param localSize The size of the memory area to map.
 * \param memType The type of memory this is allocated from.
 * \return Returns the virtual address on the remote core.
 * \ingroup group_dvp_rpc
 */
DVP_PTR dvp_rpc_mem_xlate_fwrd(DVP_RPC_t *rpc,
                               DVP_Core_e core,
                               DVP_PTR localAddr,
                               DVP_U32 localSize,
                               DVP_MemType_e memType);

/*!
 * \brief This function translates a remote virtual address into a local
 * virtual address.
 * \param rpc The pointer to the RPC context.
 * \param core The remote core to get the virtual address for.
 * \param remoteAddr The remote irtual address.
 * \param memType The type of memory this is allocated from.
 * \return Returns the virtual address on the local core.
 * \ingroup group_dvp_rpc
 */
DVP_PTR dvp_rpc_mem_xlate_back(DVP_RPC_t *rpc,
                               DVP_Core_e core,
                               DVP_PTR remoteAddr,
                               DVP_MemType_e memType);

/*!
 * \brief This function removes the local pointer from all remote translation
 * hashes.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] localAddr
 * \param [in] localSize
 * \param [in] memType
 * \ingroup group_dvp_rpc
 */
void dvp_rpc_mem_remove(DVP_RPC_t *rpc,
                        DVP_PTR localAddr,
                        DVP_U32 localSize,
                        DVP_MemType_e memType);

/*!
 * \brief This function initializes the interfaces for a particular remote
 * core (remote calls and local callbacks).
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] rpci
 * \ingroup group_dvp_rpc
 */
DVP_RPC_Core_t *dvp_rpc_core_init(DVP_RPC_t *rpc, DVP_RPC_Interface_t *rpci);

/*!
 * \brief This function deinitializes the interface to the remote core.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] rpcc
 * \ingroup group_dvp_rpc
 */
void dvp_rpc_core_deinit(DVP_RPC_t *rpc, DVP_RPC_Core_t **rpcc);

/*!
 * \brief This function removes the association of the pointer to the remote
 * cores.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] mem
 * \param [in] ptr
 * \param [in] pHandle
 * \param [in] size
 * \param [in] mtype
 * \ingroup group_dvp_rpc
 */
DVP_BOOL dvp_rpc_dissociate(DVP_RPC_t *rpc,
                            DVP_Mem_t *mem,
                            DVP_PTR ptr,
                            DVP_VALUE *pHandle,
                            size_t size,
                            DVP_MemType_e mtype);

/*!
 * \brief This function associates the pointer with it's handle and the
 * remote cores.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] mem
 * \param [in] ptr
 * \param [in] handle
 * \param [in] size
 * \param [in] mtype
 * \ingroup group_dvp_rpc
 */
DVP_BOOL dvp_rpc_associate(DVP_RPC_t *rpc,
                           DVP_Mem_t *mem,
                           DVP_PTR ptr,
                           DVP_VALUE handle,
                           size_t size,
                           DVP_MemType_e mtype);

/*!
 * \brief Retrieves the interal handle of the memory pointer.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] ptr
 * \ingroup group_dvp_rpc
 */
DVP_VALUE dvp_rpc_get_handle(DVP_RPC_t *rpc, DVP_PTR ptr);

/*!
 * \brief This function allows remote execution of code.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] rpcc
 * \param [in] cliIndex
 * \param [in] params
 * \param [in] numParams
 * \param [in] trans
 * \ingroup group_dvp_rpc
 */
DVP_S32 dvp_rpc_remote_execute(DVP_RPC_t *rpc,
                               DVP_RPC_Core_t *rpcc,
                               DVP_U32 cliIndex,
                               DVP_RPC_Parameter_t *params,
                               DVP_U32 numParams,
                               DVP_RPC_Translation_t *trans);

/*! \brief This function invalidates memory from the cache.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] core
 * \param [in] address
 * \param [in] size
 * \param [in] mtype
 * \ingroup group_dvp_rpc
 */
DVP_BOOL dvp_rpc_invalidate(DVP_RPC_t *rpc,
                            DVP_Core_e core,
                            DVP_PTR address,
                            DVP_U32 size,
                            DVP_MemType_e mtype);

/*! \brief This function flushes data back to RAM from the cache.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] core
 * \param [in] address
 * \param [in] size
 * \param [in] mtype
 * \ingroup group_dvp_rpc
 */
DVP_BOOL dvp_rpc_flush(DVP_RPC_t *rpc,
                       DVP_Core_e core,
                       DVP_PTR address,
                       DVP_U32 size,
                       DVP_MemType_e mtype);

/*!
 * \brief This function does an optional flush then a foward translation on
 * each plane pointer.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] core
 * \param [in] pImage
 * \param [in] cacheOp
 * \param [in] base
 * \param [in] trans
 * \ingroup group_dvp_rpc
 */
void dvp_rpc_prepare_image(DVP_RPC_t *rpc,
                           DVP_Core_e core,
                           DVP_Image_t *pImage,
                           DVP_BOOL cacheOp,
                           DVP_PTR base,
                           DVP_RPC_Translation_t *trans);

/*!
 * \brief This function does a backward translation on each plane pointer as
 * well as an optional invalidate.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] core
 * \param [in] pImage
 * \param [in] cacheOps
 * \ingroup group_dvp_rpc
 */
void dvp_rpc_return_image(DVP_RPC_t *rpc,
                          DVP_Core_e core,
                          DVP_Image_t *pImage,
                          DVP_BOOL cacheOp);

/*!
 * \brief This function does an optional flush then a foward translation on
 * each plane pointer.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] core
 * \param [in] pBuffer
 * \param [in] cacheOp
 * \param [in] base
 * \param [in] trans
 * \ingroup group_dvp_rpc
 */
void dvp_rpc_prepare_buffer(DVP_RPC_t *rpc,
                            DVP_Core_e core,
                            DVP_Buffer_t *pBuffer,
                            DVP_BOOL cacheOp,
                            DVP_PTR base,
                            DVP_RPC_Translation_t *trans);

/*!
 * \brief This function does a backward translation on each plane pointer
 * as well as an optional invalidate.
 * \param [in] rpc The pointer to the RPC context.
 * \param [in] core
 * \param [in] pBuffer
 * \param [in] cacheOp
 * \ingroup group_dvp_rpc
 */
void dvp_rpc_return_buffer(DVP_RPC_t *rpc,
                           DVP_Core_e core,
                           DVP_Buffer_t *pBuffer,
                           DVP_BOOL cacheOp);

#ifdef __cplusplus
}
#endif

#endif
