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

#include <sosal/rpc_socket.h>
#include <sosal/debug.h>

#if defined(SOSAL_RPC_SOCKET_API)

static void rpc_socket_item_print(uint32_t index, rpc_socket_item_t *item)
{
    char buffer[MAX_PATH];
    if (item->item)
    {
        switch (item->type)
        {
            case RPC_SOCKET_TYPE_INT8:
            {
                int8_t c = *(int8_t *)item->item;
                snprintf(buffer, MAX_PATH, "[%u] %d (0x%02x)", index, c, c);
                break;
            }
            case RPC_SOCKET_TYPE_UINT8:
            {
                uint8_t c = *(uint8_t *)item->item;
                snprintf(buffer, MAX_PATH, "[%u] %u (0x%02x)", index, c, c);
                break;
            }
            case RPC_SOCKET_TYPE_INT16:
            {
                int16_t c = *(int16_t *)item->item;
                snprintf(buffer, MAX_PATH, "[%u] %hd (0x%04x)", index, c, c);
                break;
            }
            case RPC_SOCKET_TYPE_UINT16:
            {
                uint16_t c = *(uint16_t *)item->item;
                snprintf(buffer, MAX_PATH, "[%u] %hu (0x%04x)", index, c, c);
                break;
            }
            case RPC_SOCKET_TYPE_INT32:
            {
                int32_t c = *(int32_t *)item->item;
                snprintf(buffer, MAX_PATH, "[%u] %d (0x%08x)", index, c, c);
                break;
            }
            case RPC_SOCKET_TYPE_UINT32:
            {
                uint32_t c = *(uint32_t *)item->item;
                snprintf(buffer, MAX_PATH, "[%u] %u (0x%08x)", index, c, c);
                break;
            }
            case RPC_SOCKET_TYPE_INT64:
            {
                int64_t c = *(int64_t *)item->item;
                snprintf(buffer, MAX_PATH, "[%u] "FMT_INT64_T" ("FMT_HEX64_T")", index, c, c);
                break;
            }
            case RPC_SOCKET_TYPE_UINT64:
            {
                uint64_t c = *(uint64_t *)item->item;
                snprintf(buffer, MAX_PATH, "[%u] "FMT_UINT64_T" ("FMT_HEX64_T")", index, c, c);
                break;
            }
            default:
                snprintf(buffer, MAX_PATH, "unknown");
                break;
        }
    }
    else
        snprintf(buffer, MAX_PATH, "(nil)");
    SOSAL_PRINT(SOSAL_ZONE_RPC, "\t[%u] Parameter Type %u, size: %u, num: %u, value: %s\n",
        index, item->type, item->sizeElement, item->numElements, buffer);
}

static void rpc_socket_packet_print(char *prefix, uint8_t *packet, uint32_t len)
{
    static char buffer[(RPC_SOCKET_MAX_PACKET*3)+1]; // converting BYTES to NIBBLES+SPACE to print
    static char hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    uint32_t i;
    // convert each byte into a HEX character
    for (i = 0; i < len; i++)
    {
        uint8_t bn = packet[i]; // bottom nibble
        uint8_t tn = packet[i]; // top nibble
        tn >>= 4; // adjust
        buffer[i*3 + 0] = hex[tn&0xF];
        buffer[i*3 + 1] = hex[bn&0xF];
        buffer[i*3 + 2] = ' ';
    }
    buffer[i*3] = '\0';
    prefix = prefix; // warnings
    SOSAL_PRINT(SOSAL_ZONE_RPC, "%s PACKET[%u] %s\n", prefix, len, buffer);
}

static bool_e rpc_socket_send(socket_t *socket, uint8_t *packet, size_t len, rpc_socket_header_t *header, rpc_socket_item_t *items)
{
    uint32_t i,offset = 0;

    header->length = sizeof(rpc_socket_header_t);

    // concatenate all the items to the length
    for (i = 0; i < header->numItems; i++)
    {
        rpc_socket_item_length(&items[i]); // fill in the correct length value;
        header->length += items[i].length;
    }

    // copy all data into the packet.
    memcpy(&packet[offset], header, sizeof(rpc_socket_header_t));
    offset += sizeof(rpc_socket_header_t);
    for (i = 0; i < header->numItems; i++)
    {
        if ((offset + items[i].length) <= len)
        {
            memcpy(&packet[offset], &items[i], RPC_SOCKET_ITEM_HEADER_SIZE);
            offset += RPC_SOCKET_ITEM_HEADER_SIZE;
            memcpy(&packet[offset], items[i].item, items[i].sizeElement*items[i].numElements);
            offset += (items[i].sizeElement*items[i].numElements);
        }
        else
        {
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR: RPC: Packet Length of "FMT_SIZE_T" is insufficient to hold the needed marshalled data of %u\n", len, offset);
            return false_e; // too much info to send over the packet
        }
    }

    SOSAL_PRINT(SOSAL_ZONE_RPC, "SEND(%u) function: %u numItems: %u\n", offset, header->function, header->numItems);
    rpc_socket_packet_print("SEND", packet, offset);
    return socket_send(socket, packet, offset);
}

static bool_e rpc_socket_recv(socket_t *socket, uint8_t *packet, size_t len, rpc_socket_header_t *header, rpc_socket_item_t *items)
{
    uint32_t i,offset = 0;

    if (socket_recv(socket, (uint8_t *)&offset, sizeof(uint32_t)) == true_e) // get the length of the packet
    {
        if (offset > len)
        {
            /// @todo we can't read this out, ... error?
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "CATASTROPHIC ERROR! PACKET WAS TOO SMALL ("FMT_SIZE_T") TO HOLD INCOMING DATA (%u)\n",len, offset);
            return false_e;
        }
        *(uint32_t *)&packet[0] = offset; // save offset in the packet too.
        if (socket_recv(socket, &packet[sizeof(uint32_t)], offset-sizeof(uint32_t)) == true_e)
        {
            SOSAL_PRINT(SOSAL_ZONE_RPC, "Received a Packet of %u bytes \n", offset);
            memcpy(header, &packet[0], sizeof(rpc_socket_header_t));
            header->length = offset; // remember we read this out first...
            offset = sizeof(rpc_socket_header_t); // reset the offset
            SOSAL_PRINT(SOSAL_ZONE_RPC, "Packet requests Function %u with %u parameters\n", header->function, header->numItems);
            for (i = 0; i < header->numItems; i++)
            {
                uint32_t len = 0;
                memcpy(&items[i], &packet[offset], RPC_SOCKET_ITEM_HEADER_SIZE);
                offset += RPC_SOCKET_ITEM_HEADER_SIZE;
                len = items[i].numElements * items[i].sizeElement;
                if (len > 0 && items[i].item == NULL)  // server thread
                    items[i].item = &packet[offset]; // no copy needed! the packet is persistent across the call.
                else // client call, where they've already allocated the memory
                    memcpy(items[i].item, &packet[offset], len);
                rpc_socket_item_print(i, &items[i]);
                offset += len;
            }
            rpc_socket_packet_print("RECV",packet,offset);
            SOSAL_PRINT(SOSAL_ZONE_RPC, "RECV(%u) function: %u numItems: %u\n", header->length, header->function, header->numItems);
            return true_e;
        }
        else
            return false_e;
    }
    else
        return false_e;
}

//******************************************************************************
// SERVER SIDE
//******************************************************************************

void rpc_socket_server_deinit(rpc_server_t *server)
{
    if (server)
    {
        //socket_disconnect(server->server);
        socket_deinit(server->server);
        free(server->functions);
        free(server);
    }
}

rpc_server_t *rpc_socket_server_init(char *hostname, uint16_t port, void *arg, rpc_server_function_f *functions, uint32_t numFunctions)
{
    rpc_server_t *server = (rpc_server_t *)calloc(1, sizeof(rpc_server_t));
    if (server)
    {
        server->server = socket_init(socket_hostname(hostname), port, SOCKET_TCP, true_e);
        server->numFunctions = numFunctions;
        server->port = port;
        server->arg = arg;
        server->functions = (rpc_server_function_f *)calloc(numFunctions, sizeof(rpc_server_function_f));
        if (server->functions)
        {
            memcpy(server->functions, functions, sizeof(rpc_server_function_f)*numFunctions);
        }
    }
    return server;
}

static thread_ret_t rpc_socket_server_thread(void *arg)
{
    rpc_server_t *server = (rpc_server_t *)arg;
    uint8_t packet[RPC_SOCKET_MAX_PACKET];
    rpc_socket_header_t header;
    rpc_socket_item_t   items[RPC_SOCKET_MAX_ARGS];
    rpc_socket_item_t   returns[RPC_SOCKET_MAX_ARGS]; // these fields will be copied from local static data
    uint32_t numReturns;
    bool_e ret = false_e;
    uint32_t i = 0;

    SOSAL_PRINT(SOSAL_ZONE_RPC, "%s started! Packet has "FMT_CONST" bytes\n",__FUNCTION__, RPC_SOCKET_MAX_PACKET);

    while(server->running == true_e)
    {
        SOSAL_PRINT(SOSAL_ZONE_RPC, "Starting RPC Server on port %u\n", server->port);
        // wait for a connection...
        if (socket_connect(server->server) == true_e)
        {
            SOSAL_PRINT(SOSAL_ZONE_RPC, "Server connected to Client on port %u\n", server->port);
            do {
                // clear out the previous iteration
                memset(&header, 0, sizeof(header));
                memset(items, 0, sizeof(items));
                memset(returns, 0, sizeof(returns));
                memset(packet, 0, sizeof(packet));

                SOSAL_PRINT(SOSAL_ZONE_RPC, "Server waiting for Remote Procedure Call\n");
                ret = rpc_socket_recv(server->server, packet, sizeof(packet), &header, items);
                if (ret == true_e && header.function < server->numFunctions && server->functions[header.function] != NULL)
                {
                    SOSAL_PRINT(SOSAL_ZONE_RPC, "Server Calling Function[%u]=%p with %u parameters\n", header.function, server->functions[header.function], header.numItems);
                    for (i = 0; i < header.numItems; i++)
                    {
                        rpc_socket_item_print(i, &items[i]);
                    }
                    ret = server->functions[header.function](server->arg, items, header.numItems, returns, &numReturns);
                    header.direction = RPC_SOCKET_RETURN;
                    header.numItems = numReturns;
                    // keep the function ID
                    // length will be recalculated in send
                    if (ret == false_e)
                    {
                        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR: RPC: Desired function %u did not accept parameter list\n", header.function);
                        header.numItems = 0;
                    }
                    ret = rpc_socket_send(server->server, packet, sizeof(packet), &header, returns);
                }
                else
                {
                    SOSAL_PRINT(SOSAL_ZONE_RPC, "Server Socket was closed!\n");
                }
            } while (ret == true_e);

            // we assume that all items data fields must be freed
            for (i = 0; i < dimof(items); i++)
                if (items[i].item)
                    free(items[i].item);
        }
        else
        {
            SOSAL_PRINT(SOSAL_ZONE_RPC, "RPC Socket Server could not server on port %u!\n", server->port);
            break;
        }
    }
    SOSAL_PRINT(SOSAL_ZONE_RPC, "RPC Socket Server Closing!\n");
    thread_exit(0);
}

bool_e rpc_socket_server_start(rpc_server_t *server)
{
    if (server && server->handle == 0)
    {
        server->running = true_e;
        server->handle = thread_create(rpc_socket_server_thread, server);
        if (server->handle != 0)
            return true_e;
        else
            return false_e;
    }
    return false_e;
}

void rpc_socket_server_stop(rpc_server_t *server)
{
    if (server)
    {
        server->running = false_e;
        if (server->server)
            socket_disconnect(server->server);
        SOSAL_PRINT(SOSAL_ZONE_RPC, "Closed Server Socket\n");
        if (server->handle)
            thread_join(server->handle);
    }
}

//******************************************************************************
// CLIENT SIDE
//******************************************************************************

void rpc_socket_client_stop(rpc_client_t *client)
{
    if (client)
    {
        socket_disconnect(client->server);
        socket_deinit(client->server);
        free(client);
    }
}

rpc_client_t *rpc_socket_client_start(char *hostname, uint16_t port)
{
    rpc_client_t *client = (rpc_client_t *)calloc(1, sizeof(rpc_client_t));
    if (client)
    {
        client->ipaddr = socket_hostname(hostname);
        client->port = port;
        client->server = socket_init(client->ipaddr, client->port, SOCKET_TCP, false_e);
        if (socket_connect(client->server) == true_e)
        {
            // we're good to go.
            /// @todo any validation needed?
            SOSAL_PRINT(SOSAL_ZONE_RPC, "Client Connected to Server on port %u\n", port);
        }
        else
        {
            socket_deinit(client->server);
            free(client);
            client = NULL;
        }
    }
    return client;
}

bool_e rpc_socket_client_call(rpc_client_t *client, uint32_t function, rpc_socket_item_t *items, uint32_t numItems, rpc_socket_item_t *rets, uint32_t numRets)
{
    rpc_socket_header_t header = {sizeof(rpc_socket_header_t), // more will be added later
                                  function,
                                  RPC_SOCKET_CALLING,
                                  numItems};
    uint8_t packet[RPC_SOCKET_MAX_PACKET];

    SOSAL_PRINT(SOSAL_ZONE_RPC, "Client Calling Server Function %u with %u parameters, expecting %u returns\n", function, numItems, numRets);

    if (rpc_socket_send(client->server, packet, sizeof(packet), &header, items) == true_e)
    {
        header.direction = RPC_SOCKET_RETURN;
        header.numItems = numRets;
        // function should be the same
        // length will be overwritten
        return rpc_socket_recv(client->server, packet, sizeof(packet), &header, rets);
    }
    else
        return false_e;
}

void rpc_socket_item_length(rpc_socket_item_t *item)
{
    item->length = RPC_SOCKET_ITEM_HEADER_SIZE + (item->numElements * item->sizeElement);
}

//******************************************************************************
// UNIT TEST FUNCTIONS
//******************************************************************************

#include <sosal/semaphore.h>
#define RPC_SOCKET_TEST_PORT    (5688)

static semaphore_t sem;

static int32_t function1(int32_t a, int32_t b)
{
    return a+b;
}

static int32_t function2(char *string)
{
    // don't use SOSAL_PRINT
    return printf("%s", string);
}

static bool_e rpc_server_function1(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns)
{
    static int32_t a = 0, b = 0, c = 0;
    static rpc_socket_item_t local_returns[1] = {
       {0,
        RPC_SOCKET_TYPE_INT32,
        sizeof(int32_t),
        1,
        &c},
    };
    semaphore_t *psem = (semaphore_t *)arg;

    if (numParams == 2 &&
        params[0].type == RPC_SOCKET_TYPE_INT32 &&
        params[1].type == RPC_SOCKET_TYPE_INT32)
    {
        a = *(int32_t *)params[0].item;
        b = *(int32_t *)params[1].item;
        semaphore_wait(psem);
        c = function1(a,b);
        semaphore_post(psem);
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Local Function1 Called with %u,%u returned %u\n", a,b,c);
        memcpy(returns, local_returns, sizeof(local_returns));
        *numReturns = dimof(local_returns);
        return true_e;
    }
    else
        return false_e;
}

static bool_e rpc_server_function2(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns)
{
    static int32_t c = 0;
    static rpc_socket_item_t local_returns[1] = {
       {0,
        RPC_SOCKET_TYPE_INT32,
        sizeof(int32_t),
        1,
        &c},
    };
    semaphore_t *psem = (semaphore_t *)arg;

    if (numParams == 1 &&
        params[0].type == RPC_SOCKET_TYPE_STRING)
    {
        char buffer[MAX_PATH];
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, params[0].item, params[0].numElements * params[0].sizeElement);
        semaphore_wait(psem);
        c = function2(buffer);
        semaphore_post(psem);
        memcpy(returns, local_returns, sizeof(local_returns));
        *numReturns = dimof(local_returns);
        return true_e;
    }
    else
        return false_e;
}

bool_e rpc_unittest(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    bool_e ret = false_e;
    rpc_server_function_f functions[] = {rpc_server_function1,
                                         rpc_server_function2};
    rpc_client_t *client = NULL;
    rpc_server_t *server = NULL;
    semaphore_create(&sem, 1, false_e);
    server = rpc_socket_server_init("localhost",RPC_SOCKET_TEST_PORT, &sem, functions, dimof(functions));
    if (server)
    {
        rpc_socket_server_start(server);
        thread_msleep(1000);
        client = rpc_socket_client_start("localhost",RPC_SOCKET_TEST_PORT);
        if (client)
        {
            int32_t a = 4, b = 10, c;
            char buffer[MAX_PATH];
            rpc_socket_item_t items[] = {{sizeof(rpc_socket_item_t) - 4 + sizeof(int32_t),
                                          RPC_SOCKET_TYPE_INT32,
                                          sizeof(int32_t),
                                          1,
                                          &a},
                                         {sizeof(rpc_socket_item_t) - 4 + sizeof(int32_t),
                                          RPC_SOCKET_TYPE_INT32,
                                          sizeof(int32_t),
                                          1,
                                          &b}};
            rpc_socket_item_t rets[] = {{sizeof(rpc_socket_item_t) - 4 + sizeof(int32_t),
                                         RPC_SOCKET_TYPE_INT32,
                                         sizeof(int32_t),
                                         1,
                                         &c}};
            rpc_socket_item_t stritem;
            rpc_socket_client_call(client, 0, items, dimof(items), rets, dimof(rets));
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Client is calling remote procedures!\n");
            ret = true_e;
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Received Data %u, should have been %u\n", c, a+b);
            if (c != a+b)
                ret = false_e;
            sprintf(buffer, "%i+%i=%i\n", a,b,c);
            c = 0;
            stritem.length = sizeof(rpc_socket_item_t) - 4 + strlen(buffer);
            stritem.type = RPC_SOCKET_TYPE_STRING;
            stritem.numElements = strlen(buffer);
            stritem.sizeElement = 1;
            stritem.item = buffer;
            rpc_socket_client_call(client, 1, &stritem, 1, rets, dimof(rets));
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Remote Function Printed %u characters\n", c);
            if (c == 0)
                ret = false_e;
            rpc_socket_client_stop(client);
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Client finished\n");
        }
        rpc_socket_server_stop(server);
        rpc_socket_server_deinit(server);
    }
    semaphore_delete(&sem);
    return ret;
}

#endif
