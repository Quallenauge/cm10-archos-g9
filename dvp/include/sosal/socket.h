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

#ifndef _SOSAL_SOCKET_H_
#define _SOSAL_SOCKET_H_

/*! \file  
 * \brief The SOSAL Socket API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>

/*! \brief This creates 32 address on little endian machines.  
 * \ingroup group_sockets
 */
#define IP4_ADDR(a,b,c,d)   ((d << 0) | (c << 8) | (b << 16) | (a << 24))

#define SOSAL_SOCKET_API

#if defined(ANDROID) || defined(LINUX) || defined(DARWIN) || defined(__QNX__) || defined(CYGWIN)
#define POSIX_SOCKETS
#undef WIN32_SOCKETS
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#define SOCKET_IP_ANY   INADDR_ANY
#elif defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#undef POSIX_SOCKETS
#define WIN32_SOCKETS
#define SOCKET_IP_ANY   0
#if defined(WIN32) && !defined(CYGWIN)
typedef SSIZE_T ssize_t;
#endif
#elif defined(SYSBIOS)
#undef POSIX_SOCKETS
#undef WIN32_SOCKETS
#undef SOSAL_SOCKET_API
#endif

/*! \brief This serializes the unit into the serializing buffer if the buffer has room.
 * \param [in,out] buffer The serialized buffer. 
 * \param [in] offset The offset in the buffer to start
 * \param [in] len the length of the buffer.
 * \param [in] unit The unit to write into the serialized buffer.
 * \ingroup group_sockets
 */
#define SERIALIZE_UNIT(buffer, offset, len, unit) { \
    if ((offset + sizeof(unit)) < len) { \
        memcpy(&buffer[offset], &unit, sizeof(unit)); \
        offset += sizeof(unit); \
    } \
}

/*! \brief This unserializes the unit from the serializing buffer to the unit.
 * \param [in] buffer The serialized buffer. 
 * \param [in] offset The offset in the buffer to start
 * \param [in] len the length of the buffer.
 * \param [in] unit The unit to read from the serialized buffer.
 * \ingroup group_sockets
 */
#define UNSERIALIZE_UNIT(buffer, offset, len, unit) { \
    if ((offset + sizeof(unit)) < len) { \
        memcpy(&unit, &buffer[offset], sizeof(unit)); \
        offset += sizeof(unit); \
    } \
}

/*! \brief This serializes the pointer into the serializing buffer if the buffer has room.
 * \param [in,out] buffer The serialized buffer. 
 * \param [in] offset The offset in the buffer to start
 * \param [in] len the length of the buffer.
 * \param [in] ptr The pointer to serialize from into the buffer.
 * \param [in] ptrlen The length of the ptr to serialize.
 * \ingroup group_sockets
 */
#define SERIALIZE_ARRAY(buffer, offset, len, ptr, ptrlen) { \
    if ((offset + ptrlen) < len) { \
        memcpy(&buffer[offset], ptr, ptrlen); \
        offset += ptrlen; \
    } \
}

/*! \brief This unserializes the pointer from the serializing buffer.
 * \param [in] buffer The serialized buffer. 
 * \param [in] offset The offset in the buffer to start
 * \param [in] len the length of the buffer.
 * \param [in] ptr The pointer to unserialize from the buffer.
 * \param [in] ptrlen The length of the ptr to unserialize.
 * \ingroup group_sockets
 */
#define UNSERIALIZE_ARRAY(buffer, offset, len, ptr, ptrlen) { \
    if ((offset + ptrlen) < len) { \
        memcpy(ptr, &buffer[offset], ptrlen); \
        offset += ptrlen; \
    } \
}

/*! \brief The definition of a local address.
 * \ingroup group_sockets
 */
#define SOCKET_IP_LOCAL IP4_ADDR(127,0,0,1)

#if defined(SOSAL_SOCKET_API)

/*! \brief The socket structure which maintains the HLOS specific information. 
 * \ingroup group_sockets
 */
typedef struct _socket_t {
    bool_e is_server;                   /*!< \brief The socket is a server endpoint */
#if defined(POSIX_SOCKETS)              
    int sock;                           /*!< \brief The socket handle */
    int connected;                      /*!< \brief The connected socket handle. */
    struct sockaddr_in server_address;  /*!< \brief The server address used to run the server connection. */
    struct sockaddr_in client_address;  /*!< \brief The client address used to connect to the server. */
    socklen_t sin_size;                  /*!< \brief The socket input size */
#elif defined(WIN32_SOCKETS)
    SOCKET sock;                        /*!< \brief The socket handle */
    SOCKET connected;                   /*!< \brief The connected socket handle */
    struct sockaddr_in server_address; /*!< \brief The server address used to run the server connection */
    struct sockaddr_in client_address; /*!< \brief The client address used to connect to the server. */
    int sin_size;                       /*!< \brief The socket input size */
#endif
    ssize_t bytesRecv;                  /*!< \brief The number of bytes received on the socket */
    ssize_t bytesSend;                  /*!< \brief The number of bytes sent on the socket. */
} socket_t;

/*! \brief The type of socket connection.
 * \ingroup group_sockets
 */
typedef enum _socket_type_e {
    SOCKET_TCP,     /*!< \brief Makes a TCP/IP connection */
    SOCKET_UDP,     /*!< \brief Makes a UDP/IP connection */
} socket_type_e;


#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Converts the string hostname to an ipaddress stored in the return value. 
 * \param [in] string The string of the hostname to convert.
 * \return Return the IP address in a uint32_t.
 * \ingroup group_sockets
 */
uint32_t socket_hostname(char *string);

/*! \brief Creates the server socket, binds and listens to the port, but does not yet accept connections.
 * \param [in] ipaddr The IP address.
 * \param [in] port The IP port. 
 * \param [in] type The type of connection requested.
 * \param [in] isServer Indicates that this socket will be a server style connection if true_e.
 * \post \ref socket_connect
 * \ingroup group_sockets
 */
socket_t *socket_init(uint32_t ipaddr, uint16_t port, socket_type_e type, bool_e isServer);

/*! \brief A blocking call waiting for a server/client connection.
 * \param [in] socket The socket handle. 
 * \retval true_e Returned when the connection is complete.
 * \retval false_e Returned when the connection failed.
 * \pre \ref socket_init
 * \ingroup group_sockets
 */
bool_e socket_connect(socket_t *socket);

/*! \brief This sends data over a connected socket.
 * \param [in] socket The socket handle.
 * \param [in] data The data to send over the socket.
 * \param [in] len The length of data to send from data.
 * \retval true_e The data was sent correctly.
 * \retval false_e The data was not sent correctly.
 * \pre \ref socket_connect
 * \post \ref socket_disconnect
 * \ingroup group_sockets
 */
bool_e socket_send(socket_t *socket, uint8_t *data, size_t len);

/*! \brief This receives data over the socket.
 * \param [in] socket The socket handle.
 * \param [in] data The data to received over the socket.
 * \param [in] len The length of data to receive over the socket.
 * \retval true_e The data was received correctly.
 * \retval false_e The data was not received correctly. If the connection 
 * was lost this will be returned.
 * \pre \ref socket_connect
 * \ingroup group_sockets
 */
bool_e socket_recv(socket_t *socket, uint8_t *data, size_t len);

/*! \brief This disconnects a socket from the remote side.
 * \param [in] socket The socket handle.
 * \pre \ref socket_connect
 * \ingroup group_sockets
 */
void socket_disconnect(socket_t *socket);

/*! \brief This prints information about a socket.
 * \param [in] socket The socket handle.
 * \ingroup group_sockets
 */
void socket_info(socket_t *socket);

/*! \brief This deinitializes the socket.
 * \param [in] socket The socket handle.
 * \pre \ref socket_init
 * \ingroup group_sockets
 */
void socket_deinit(socket_t *socket);

#ifdef __cplusplus
}
#endif

#endif // SOSAL_SOCKET_API

#endif

