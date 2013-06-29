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

#ifndef _RPC_SOCKET_H_
#define _RPC_SOCKET_H_

/*! \file  
 * \brief The SOSAL Remote Procedure Call API over Sockets.
 * \author Erik Rainey <erik.rainey@ti.com>
 */
 
#include <sosal/types.h>
#include <sosal/thread.h>
#include <sosal/socket.h>

#define SOSAL_RPC_SOCKET_API

#if defined(SYSBIOS)
#undef SOSAL_RPC_SOCKET_API
#endif

#if defined(SOSAL_RPC_SOCKET_API)

/*! \brief 
 * \ingroup group_rpc_sockets
 */
typedef struct _rpc_socket_item_t {
    uint32_t length;
    uint32_t type;          /**< @see rpc_socket_item_type_e */
    uint32_t sizeElement;
    uint32_t numElements;
    void    *item;          // this element is not in the actual packet data
} rpc_socket_item_t;

/*! \brief 
 * \ingroup group_rpc_sockets
 */
#define RPC_SOCKET_ITEM_HEADER_SIZE (sizeof(rpc_socket_item_t)-sizeof(void *))

/*! \brief 
 * \ingroup group_rpc_sockets
 */
typedef struct _rpc_client_t {
    uint32_t  ipaddr;
    uint16_t  port;
    socket_t *server;
} rpc_client_t;

/*! \brief 
 * \ingroup group_rpc_sockets
 */
typedef bool_e (*rpc_server_function_f)(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns);

/*! \brief 
 * \ingroup group_rpc_sockets
 */
typedef struct _rpc_server_t {
    socket_t *server;
    thread_t handle;
    uint16_t port;
    void *arg;
    rpc_server_function_f *functions;
    uint32_t numFunctions;
    bool_e running;
} rpc_server_t;

/*! \brief 
 * \ingroup group_rpc_sockets
 */
typedef struct _rpc_socket_header_t {
     uint32_t length;
     uint32_t function;
     uint32_t direction;
     uint32_t numItems;
} rpc_socket_header_t;

/*! \brief 
 * \ingroup group_rpc_sockets
 */
typedef enum _rpc_socket_direction_e {
    RPC_SOCKET_CALLING,
    RPC_SOCKET_RETURN
} rpc_socket_direction_e;

/*! \brief 
 * \ingroup group_rpc_sockets
 */
typedef enum _rpc_socket_item_type_e {
     RPC_SOCKET_TYPE_INT8,
     RPC_SOCKET_TYPE_UINT8,
     RPC_SOCKET_TYPE_INT16,
     RPC_SOCKET_TYPE_UINT16,
     RPC_SOCKET_TYPE_INT32,
     RPC_SOCKET_TYPE_UINT32,
     RPC_SOCKET_TYPE_INT64,
     RPC_SOCKET_TYPE_UINT64,
     RPC_SOCKET_TYPE_STRING
} rpc_socket_item_type_e;

/*! \brief Indicates the maximum number of items in a packet
 * \ingroup group_rpc_sockets
 */
#define RPC_SOCKET_MAX_ARGS     (10)

/*! \brief Indicates the maximum size of an item's data
 * \ingroup group_rpc_sockets
 */
#define RPC_SOCKET_MAX_DATA     (128)

/*! \brief The maximum size of an RPC packet. 
 * \note Large data should not be sent over RPC command packets. It should be 
 * arranged over a secondary connection.
 * \ingroup group_rpc_sockets
 */
#define RPC_SOCKET_MAX_PACKET   (sizeof(rpc_socket_header_t) + (RPC_SOCKET_MAX_ARGS * (RPC_SOCKET_ITEM_HEADER_SIZE + RPC_SOCKET_MAX_DATA)))

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This initializes a server thread with a preset set of 
 * functions. The thread is not started until \ref rpc_socket_server_start is 
 * called.
 * \param [in] hostname The hostname of the server.
 * \param [in] port The port to serve the server on.
 * \param [in] arg The user private data.
 * \param [in] functions The list of functions serve.
 * \param [in] numFunctions The number of functions in the list.
 * \return Returns the handle to the RPC server.
 * \retval NULL Indicates that the server did not start.
 * \post \ref rpc_socket_server_deinit
 * \ingroup group_rpc_sockets
 */
rpc_server_t *rpc_socket_server_init(char *hostname, uint16_t port, void *arg, 
                                       rpc_server_function_f *functions, 
                                       uint32_t numFunctions);

/*! \brief This deinitializes a server thread.
 * \param [in] server The server to deinitialize.
 * \pre \ref rpc_socket_server_init
 * \ingroup group_rpc_sockets
 */
void          rpc_socket_server_deinit(rpc_server_t *server);

/*! \brief This starts the server thread running.
 * \param [in] server The server to start.
 * \return Returns a bool_e.
 * \retval true_e Indicates that the server started, false_e otherwise.
 * \pre \ref rpc_socket_server_init
 * \post \ref rpc_socket_server_stop
 * \ingroup group_rpc_sockets
 */
bool_e        rpc_socket_server_start(rpc_server_t *server);

/*! \brief This stops the server thread.
 * \param [in] server The server to stop.
 * \pre \ref rpc_socket_server_start
 * \post \ref rpc_socket_server_deinit
 * \ingroup group_rpc_sockets
 */
void          rpc_socket_server_stop(rpc_server_t *server);

/*! \brief This connects a client to an RPC server.
 * \param [in] hostname The name of the server.
 * \param [in] port The port number of the server.
 * \return Returns the client structure.
 * \retval NULL if the client could not connect.
 * \post \ref rpc_socket_client_call
 * \ingroup group_rpc_sockets
 * \note
 */
rpc_client_t *rpc_socket_client_start(char *hostname, uint16_t port);

/*! \brief This calls a remote procedure call on the server.
 * \param [in] client The handle to the client context.
 * \param [in] function The index of the function to call.
 * \param [in] items The list of parameters to the function.
 * \param [in] numItems The number of items in the list of items.
 * \param [in] rets The list of return values from the function.
 * \param [in] numRets The number of items in the return list of items.
 * \return Returns bool_e.
 * \retval true_e if the function was called. 
 * \retval fasle_e an error occurred.
 * \pre \ref rpc_socket_client_start
 * \ingroup group_rpc_sockets
 * \note Remember, you are making an RPC over the network. You can not pass 
 * logically pass memory pointers!
 */
bool_e rpc_socket_client_call(rpc_client_t *client, uint32_t function, 
                              rpc_socket_item_t *items, uint32_t numItems, 
                              rpc_socket_item_t *rets, uint32_t numRets);

/*! \brief This stops the client.
 * \param [in] client The client to stop.
 * \pre \ref rpc_socket_client_start
 * \ingroup group_rpc_sockets
 */
void          rpc_socket_client_stop(rpc_client_t *client);

/*! \brief Determines the length needed in the packed for this item.
 * \param [in] items The item to compute length on.
 * \ingroup group_rpc_sockets
 */
void rpc_socket_item_length(rpc_socket_item_t *item);

#ifdef __cplusplus
}
#endif

#endif // SOSAL_RPC_SOCKET_API

#endif

