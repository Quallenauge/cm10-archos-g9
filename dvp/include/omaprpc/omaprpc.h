/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

#ifndef _OMAPRPC_USR_H_
#define _OMAPRPC_USR_H_

/*! \file
 * \brief The User API of the OMAPRPC driver.
 * \author Erik Rainey <erik.rainey@ti.com>
 * \defgroup group_omaprpc OMAPRPC Driver
 */

#include <sosal/sosal.h>
#if defined(LINUX) && defined(PANDA)
typedef size_t ptrdiff_t;
#include <linux/omap_rpc.h>
#elif defined(ANDROID)
#include <linux/omap_rpc.h>
#endif

/*! Used by clients to help define their remote call enumeration. */
#define OMAPRPC_REMOTE_FUNC_BASE (0)

// forward definition
struct _omaprpc_t;

/*! \brief The callback prototype used when the remote side restarts unexpectedly.
 * \ingroup group_omaprpc
 */
typedef void (*omaprpc_endpoint_restarted_f)(struct _omaprpc_t *rpc, void *cookie);

/*! \brief The OMAPRPC Context
 * \ingroup group_omaprpc
 */
typedef struct _omaprpc_t {
    int32_t  device;                            /*!< \brief The file descriptor of the opened device */
    char dev_name[MAX_PATH];                    /*!< \brief The local device node to open */
    struct omaprpc_create_instance_t create;    /*!< \brief The remote server name to connect to */
    uint32_t retries;                           /*!< \brief The number of tries to re-open the driver after an unexpected restart */
    uint32_t num_funcs;                         /*!< \brief The number of supported functions on the endpoint */
    struct omaprpc_call_function_t *functions;  /*!< \brief The array of supported functions on the endpoint */
    omaprpc_endpoint_restarted_f restart;       /*!< \brief The function to call if the remote endpoint restarts unexpectedly. */
    bool_e in_restart;                          /*!< \brief This indicates that the driver is in a restart state */
    void *cookie;                               /*!< \brief A user supplied context pointer to be passed to the restart callback */
#if defined(OMAPRPC_USE_ION)
    list_t fd_list;
#endif
} omaprpc_t;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief This function opens the local device node which connects to a remote core with a particular service.
 * \param [in] device_name The device to open. i.e.: "/dev/omaprpc-0"
 * \param [in] server_name The name of the service to connect to. i.e.: "omaprpc-0"
 * \param [in] numFuncs The number of functions that the service has.
 * \ingroup group_omaprpc
 */
omaprpc_t *omaprpc_open(char *device_name, char *server_name, uint32_t numFuncs);

/*!
 * \brief This function closes an OMAPRPC device opened by omaprpc_open.
 * \pre \ref omaprpc_open
 * \param [in,out] rpm The pointer to the omaprpc_t pointer. This will be NULLed by
 * this function.
 * \ingroup group_omaprpc
 */
void omaprpc_close(omaprpc_t **rpm);

/*!
 * \brief This function registers a callback to be called if the remote endpoint restarts unexpectedly.
 * \param rpc The handle to the OMAPRPC context.
 * \param cookie A pointer to some user private context data.
 * \param func The function to call when the restart occurs.
 * \param retries The number of times to retry opening the driver after an unexpected restart.
 * \ingroup group_omaprpc
 */
void omaprpc_restart_callback(omaprpc_t *rpc, void *cookie, omaprpc_endpoint_restarted_f func, uint32_t retries);

/*!
 * \brief This function calls a remote function and can get a status code
 * from that operation.
 * \param [in] rpc The handle to the OMARPPC context.
 * \param [in] function The function descriptor used to call the remote function.
 * \param [in] returned The return value from the called function. This can be NULL if you dont' care about the return value.
 * \ingroup group_omaprpc
 */
bool_e omaprpc_call(omaprpc_t *rpc, struct omaprpc_call_function_t *function, struct omaprpc_function_return_t *returned);

#if defined(OMAPRPC_USE_ION)
/*!
 * \brief This function registers a piece of memory with OMAPRPC so that if
 * this process dies, the kernel will still retain a handle to that memory, thus
 * preventing the memory from being freed and removed from remote cores asynchronously.
 * \note This is only required for ION memory.
 * \param [in] rpc The handle to the OMAPRPC context.
 * \param [in] memdevice The file descriptor of the opened "/dev/ion".
 * \param [in] ptr The user virtual address of the pointer to register.
 * \param [in] reserved The ION handle to the memory.
 * \ingroup group_omaprpc
 */
#endif
bool_e omaprpc_register(omaprpc_t *rpc, int memdevice, void *ptr, void **reserved);

#if defined(OMAPRPC_USE_ION)
/*!
 * \brief This function unregisters a piece of memory with OMAPRPC and removes
 * the reference in the kernel.
 * \note This is only required for ION memory.
 * \param [in] rpc The handle to the OMAPRPC context.
 * \param [in] memdevice The file descriptor of the opened "/dev/ion".
 * \param [in] ptr The user virtual address of the pointer to register.
 * \param [in] reserved The ION handle to the memory.
 * \ingroup group_omaprpc
 */
#endif
bool_e omaprpc_unregister(omaprpc_t *rpc, int memdevice, void *ptr, void **reserved);

#ifdef __cplusplus
}
#endif

#endif
