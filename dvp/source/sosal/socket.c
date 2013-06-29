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

#include <sosal/debug.h>
#include <sosal/socket.h>

#if defined(SOSAL_SOCKET_API)

#if defined(WIN32_SOCKETS)

#define SOCKET_CLOSE(sock)     closesocket(sock)
#define SOCKET_GETERROR        WSAGetLastError()
static bool_e winsockinit = false_e;
#define SOCKET_INIT() { \
    if (winsockinit == false_e) { \
        WSADATA data; \
        WSAStartup(2|2<<4, &data); \
        winsockinit = true_e; \
    } \
}
#define SOCKET_DEINIT()        WSACleanup()
#define SOCKET_OPTCAST         char *

#else

#define SOCKET_CLOSE(sock)     close(sock)
#define SOCKET_GETERROR        errno
#define SOCKET_INIT()
#define SOCKET_DEINIT()
#define SOCKET_ERROR           (-1)
#define INVALID_SOCKET         (-1)
#define SOCKET_OPTCAST         void *

#endif

socket_t *socket_init(uint32_t ipaddr, uint16_t port, socket_type_e type, bool_e isServer)
{
    SOCKET_INIT();
    if (isServer && type == SOCKET_TCP)
    {
        socket_t *server = calloc(1, sizeof(socket_t));
        if (server)
        {
            int ret = 1;
            server->is_server = true_e;
            server->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (server->sock == INVALID_SOCKET)
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to open a server socket\n");
                goto server_failure;
            }

            if (setsockopt(server->sock, SOL_SOCKET, SO_REUSEADDR, (SOCKET_OPTCAST)&ret, sizeof(int)) == SOCKET_ERROR)
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to set socket options\n");
                goto server_failure;
            }

            memset(&server->server_address, 0, sizeof(struct sockaddr_in));
            server->server_address.sin_family = AF_INET;
            server->server_address.sin_port = htons(port);
            server->server_address.sin_addr.s_addr = ipaddr;
#if defined(ANDROID)
            memset(&(server->server_address.__pad), 0, sizeof(server->server_address.__pad));
#endif

            if (bind(server->sock, (struct sockaddr *)&server->server_address, sizeof(struct sockaddr)) == SOCKET_ERROR)
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to bind to socket\n");
                goto server_failure;
            }

            if (listen(server->sock, 1) == SOCKET_ERROR)
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to listen on socket\n");
                goto server_failure;
            }

            SOSAL_PRINT(SOSAL_ZONE_SOCKET, "Created Socket Server on Port %u\n", port);
            return server;
server_failure:
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to init server\n");
            if (server->sock != SOCKET_ERROR)
                SOCKET_CLOSE(server->sock);
            free(server);
            server = NULL;
        }
    }
    else
    {
        socket_t *client = calloc(1, sizeof(socket_t));
        if (client)
        {
            client->is_server = false_e;

            client->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (client->sock == INVALID_SOCKET)
                goto client_failure;

            memset(&client->server_address, 0, sizeof(struct sockaddr_in));
            client->server_address.sin_family = AF_INET;
            client->server_address.sin_port = htons(port);
            client->server_address.sin_addr.s_addr = ipaddr;
#if defined(ANDROID) || defined(CYGWIN)
            memset(&(client->server_address.__pad), 0, sizeof(client->server_address.__pad));
#endif

            SOSAL_PRINT(SOSAL_ZONE_SOCKET, "Created Socket Client %d on Port %u\n", client->sock, port);
            return client;
client_failure:
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to init client\n");
            if (client->sock != SOCKET_ERROR)
                SOCKET_CLOSE(client->sock);
            free(client);
            client = NULL;
        }
    }
    return NULL;
}

bool_e socket_connect(socket_t *socket)
{
    if (socket)
    {
        socket->sin_size = sizeof(struct sockaddr);
        if (socket->is_server == true_e)
        {
#if defined(WIN32) && !defined(CYGWIN)
           SOSAL_PRINT(SOSAL_ZONE_SOCKET, "Connecting to %u.%u.%u.%u\n",
                socket->client_address.sin_addr.s_net,
                socket->client_address.sin_addr.s_host,
                socket->client_address.sin_addr.s_lh,
                socket->client_address.sin_addr.s_impno);
#endif
            socket->connected = accept(socket->sock, (struct sockaddr *)&socket->client_address, &socket->sin_size);
            if (socket->connected == INVALID_SOCKET)
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Server failed to connect. errno=%d\n", SOCKET_GETERROR);
                return false_e;
            }
            else
            {
                SOSAL_PRINT(SOSAL_ZONE_SOCKET, "Server Connected Socket %d\n", socket->connected);
                return true_e;
            }
        }
        else
        {
#if defined(WIN32) && !defined(CYGWIN)
           SOSAL_PRINT(SOSAL_ZONE_SOCKET, "Connecting to %u.%u.%u.%u\n",
                socket->server_address.sin_addr.s_net,
                socket->server_address.sin_addr.s_host,
                socket->server_address.sin_addr.s_lh,
                socket->server_address.sin_addr.s_impno);
#endif
            if (connect(socket->sock, (struct sockaddr *)&socket->server_address, socket->sin_size) == SOCKET_ERROR)
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Client failed to connect.\n");
                return false_e;
            }
            else
                return true_e;
        }
    }
    else
        return false_e;
}

bool_e socket_send(socket_t *socket, uint8_t *data, size_t len)
{
    if (socket && data && len > 0)
    {
        ssize_t sent = 0;
        int err = 0;
        int sock = 0;
        int flags = 0;
        if (socket->is_server)
            sock = socket->connected;
        else
            sock = socket->sock;
        sent = send(sock, data, len, flags);
        err = SOCKET_GETERROR;
        if (sent == 0)
            return false_e;
        else if (sent == SOCKET_ERROR)
        {
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "SOSAL: Failed to send data on socket! send(%d, %p, "FMT_SIZE_T", %x) return "FMT_SSIZE_T" had (errno=%d)\n", sock, data, len, flags, sent, err);
            return false_e;
        }
        else
        {
            socket->bytesSend += sent;
            if (sent != (ssize_t)len) {
                SOSAL_PRINT(SOSAL_ZONE_WARNING, "SOSAL: ONLY SENT "FMT_SSIZE_T" BYTES ON SOCKET %p, WANTED "FMT_SIZE_T"\n", sent, socket, len);
            } else {
                SOSAL_PRINT(SOSAL_ZONE_SOCKET, "SOSAL: Sent "FMT_SSIZE_T" bytes on Socket %p\n", sent, socket);
            }
            return true_e;
        }
    }
    else
        return false_e;
}

bool_e socket_recv(socket_t *socket, uint8_t *data, size_t len)
{
    if (socket && data && len > 0)
    {
        ssize_t recvd = 0;
        int flags = 0;
#if defined(POSIX_SOCKETS)
        flags |= MSG_WAITALL;
#endif
        SOSAL_PRINT(SOSAL_ZONE_SOCKET, "Waiting for "FMT_SIZE_T" data on socket %p to ptr %p\n", len, socket, data);
        do {
            ssize_t _recvd = 0;
            int err = 0;
            int sock = 0;
            if (socket->is_server)
                sock = socket->connected;
            else
                sock = socket->sock;
            _recvd = recv(sock, &data[recvd], len-recvd, flags);
            err = SOCKET_GETERROR;
            if (_recvd == 0 && err != 0) {
                SOSAL_PRINT(SOSAL_ZONE_WARNING, "recv() returned zero bytes with error! (errno=%d)\n", err);
#if defined(POSIX_SOCKETS)
                if (err == EINVAL) {
                    SOSAL_PRINT(SOSAL_ZONE_WARNING, "recv(%d, %p, "FMT_SIZE_T", %08x)\n", sock, &data[recvd], len-(size_t)recvd, flags);
                }
#endif
                return false_e;
            }
            if (_recvd >= 0)
                recvd += _recvd;
            else // error
            {
                recvd = _recvd;
                break;
            }
        } while (recvd < (ssize_t)len);
        if (recvd == 0)
            return false_e;
        else if (recvd == SOCKET_ERROR)
        {
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "SOSAL: Failed to recv data on socket! (errno=%d)\n", SOCKET_GETERROR);
            return false_e;
        }
        else
        {
            socket->bytesRecv += recvd;
            if (recvd != (ssize_t)len) {
                SOSAL_PRINT(SOSAL_ZONE_WARNING, "SOSAL: ONLY RECEIVED "FMT_SSIZE_T" BYTES ON SOCKET %p, WANTED "FMT_SIZE_T"\n", recvd, socket, len);
            } else {
                SOSAL_PRINT(SOSAL_ZONE_SOCKET, "SOSAL: Recv "FMT_SSIZE_T" bytes on Socket %p\n", recvd, socket);
            }
            return true_e;
        }
    }
    else
        return false_e;
}

void socket_disconnect(socket_t * socket)
{
    if (socket)
    {
        if (socket->is_server)
            SOCKET_CLOSE(socket->connected);
        else
            SOCKET_CLOSE(socket->sock);
    }
}

void socket_deinit(socket_t *socket)
{
    if (socket)
    {
        if (socket->is_server)
            SOCKET_CLOSE(socket->sock);
        memset(socket, 0, sizeof(socket_t));
        free(socket);
    }
    SOCKET_DEINIT();
}

uint32_t socket_hostname(char *string)
{
    struct hostent *host = NULL;
    uint32_t ipaddr = 0;
    struct in_addr *addr = NULL;

    SOCKET_INIT();

    host = gethostbyname((const char *)string);
    if (host)
    {
#if defined(DARWIN)
        addr = (struct in_addr *)host->h_addr_list[0];
#else
		addr = (struct in_addr *)host->h_addr;
#endif
        if (addr)
        {
            ipaddr = addr->s_addr;
#if defined(WIN32) && !defined(CYGWIN)
            SOSAL_PRINT(SOSAL_ZONE_SOCKET, "Host name %s resolves to %u.%u.%u.%u (errno=%d)\n",
                string,
                addr->s_net,addr->s_host,addr->s_lh,addr->s_impno,
                SOCKET_GETERROR);
#endif
        }
        else
        {
            /// @todo Host address structure was NULL
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to resolve host name %s\n", string);
        }
    }
    else
    {
        /// @todo Host structure was NULLs
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to resolve host name %s\n", string);
    }
    return ipaddr;
}

void socket_info(socket_t *socket)
{
    socket = socket;
    SOSAL_PRINT(SOSAL_ZONE_SOCKET, "Send: "FMT_SSIZE_T" bytes\n", socket->bytesSend);
    SOSAL_PRINT(SOSAL_ZONE_SOCKET, "Recv: "FMT_SSIZE_T" bytes\n", socket->bytesRecv);
}

#include <sosal/sosal.h>

static event_t f_wait;
#define DATA_SIZE  (320*240*2)  // Simulate QVGA UYVY data
#define DATA_COUNT (30*10)      // Simulate 30fps for 10 seconds.

void memfill(uint8_t *ptr, size_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        ptr[i] = (uint8_t)(i & 0xFF);
    }
}

static thread_ret_t thread_server(void *arg)
{
    socket_t *server = (socket_t *)arg;
    uint8_t data[DATA_SIZE];
    uint32_t count;

    event_set(&f_wait); // tell the client that the server is about to start
    SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Server Thread Running\n");
    if (socket_connect(server) == true_e)
    {
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Server Started!\n");
        socket_recv(server, (uint8_t *)&count, sizeof(count));
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Echoing %u (0x%08x) times\n", count, count);
        while (count--)
        {
            socket_recv(server, data, sizeof(data));
            socket_send(server, data, sizeof(data));
        }
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Echo'd\n");
        socket_disconnect(server);
    }
    else
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Could not start server!\n");
    thread_exit(0);
}

static thread_ret_t thread_client(void *arg)
{
    socket_t *client = (socket_t *)arg;
    uint32_t count = DATA_COUNT;
    uint32_t size = DATA_SIZE * count;
    uint8_t data[DATA_SIZE];
    double rate = 0.0f;

    memfill(data, DATA_SIZE);
    if (event_wait(&f_wait, 1000) == true_e) // wait for the server to start
    {
        if (socket_connect(client) == true_e)
        {
            profiler_t perf;
            profiler_clear(&perf);
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Connected Client\n");

            profiler_start(&perf);
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Echoing %u (0x%08x) times\n", count, count);
            socket_send(client, (uint8_t *)&count, sizeof(count));

            while (count--)
            {
                socket_send(client, data, DATA_SIZE);
                memset(data, 0, DATA_SIZE);
                socket_recv(client, data, DATA_SIZE);
            }
            socket_info(client);
            socket_disconnect(client);
            profiler_stop(&perf);
            rate = (double)size*rtimer_freq()/perf.avgTime;
            rate /= 1000000;
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Perf: "FMT_RTIMER_T" (Rate: %.2lf MBps)\n", perf.avgTime,rate);
            if (data[1] == 0x01) {
                SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Ping/Pong SUCCESS!\n");
            } else {
                SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Ping/Pong FAILED!\n");
            }
        }
        else
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Could not connect to server!\n");
    }
    thread_exit(0);
}

static bool_e no_server;
static bool_e no_client;
static option_t opts[] = {
    {OPTION_TYPE_BOOL, &no_server, sizeof(bool_e), "-ns", "--no-server", "Disables the server"},
    {OPTION_TYPE_BOOL, &no_client, sizeof(bool_e), "-nc", "--no-client", "Disables the client"},
};

bool_e socket_unittest(int argc, char *argv[])
{
    thread_t server_thread = 0;
    thread_t client_thread = 0;
    uint32_t address = socket_hostname("127.0.0.1");
    socket_t *server = NULL;
    socket_t *client = NULL;
    bool_e ret = true_e;

    no_server = false_e;
    no_client = false_e;

    option_process(argc, argv, opts, dimof(opts));

    SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Starting Socket Test!\n");

    event_init(&f_wait, true_e);

    if (!no_server)
        server = socket_init(SOCKET_IP_ANY, 1234, SOCKET_TCP, true_e);
    else
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Server is not launched!\n");

    if (!no_client)
        client = socket_init(address, 1234, SOCKET_TCP, false_e);
    else
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Client is not launched!\n");

    if ((no_server || server) && (no_client || client))
    {
        if (!no_server)
            server_thread = thread_create(thread_server, server);
        else if (!no_client)
            event_set(&f_wait);
        if (!no_client) client_thread = thread_create(thread_client, client);

        if (!no_server) thread_join(server_thread);
        if (!no_client) thread_join(client_thread);
    }
    else
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Could not create sockets!\n");

    if (!no_server) socket_deinit(server);
    if (!no_client) socket_deinit(client);

    event_deinit(&f_wait);

    return ret;
}

#endif
