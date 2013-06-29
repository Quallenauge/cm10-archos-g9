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

#include <omaprpc/omaprpc.h>

#if defined(ANDROID)
#include <android/log.h>
#endif

#if defined(OMAPRPC_USE_ION)
#include <ion/ion.h>
#if defined(ANDROID)
#include <hwc/hal_public.h>
#endif
#endif

#define OMAPRPC_RESTART_CONDITION(ret) (errno == ENXIO || errno == ENOTCONN)

//******************************************************************************
// TYPEDEFS, ENUMS and #DEFINES
//******************************************************************************

/*! \brief The local definitions of debugging zones for OMAPRPC.
 * \ingroup group_omaprpc
 */
typedef enum _omaprpc_zone_mask_e {
    OMAPRPC_ZONE_ERROR = 0,     /*!< \brief The error zone */
    OMAPRPC_ZONE_WARNING,       /*!< \brief The warning zone for non-critical issues */
    OMAPRPC_ZONE_INFO,          /*!< \brief The info zone for general information */
    OMAPRPC_ZONE_API,           /*!< \brief The API zone for API tracing */
    OMAPRPC_ZONE_PERF,          /*!< \brief The performance information zone */

    // don't use this, it's only there to count the number of zones
    OMAPRPC_ZONE_MAX
} OMAPRPC_ZoneMask_e;

/*! \brief Used to set the \ref OMAPRPC_ZoneMask_e into a bitfield. */
#define SET_FLAG(flag)      (1<<flag)

#if defined(OMAPRPC_DEBUG)

#ifndef OMAPRPC_ZONE_MASK

static uint32_t zone_mask;

#define OMAPRPC_RUNTIME_DEBUG
/*! \brief The default debugging settings for OMAPRPC. */
#define OMAPRPC_ZONE_MASK   zone_mask
#endif

#if defined(ANDROID) && (OMAPRPC_DEBUG == 2)
/*! \brief The ANDROID specific method of debugging. */
#define OMAPRPC_PRINT(flag, format, ...) if (SET_FLAG(flag) & OMAPRPC_ZONE_MASK) { __android_log_print(ANDROID_LOG_DEBUG, #flag, format, ## __VA_ARGS__); }
#else
//#warning "OMAPRPC_PRINT is printf"
/*! \brief The generic method of debugging. */
#define OMAPRPC_PRINT(flag, format, ...) if (SET_FLAG(flag) & OMAPRPC_ZONE_MASK) { printf(format, ## __VA_ARGS__); }
#endif
#else
//#warning "OMAPRPC_PRINT is disabled"
/*! \brief Debugging is disabled. */
#define OMAPRPC_PRINT(flag, format, ...)
#endif

//******************************************************************************
// LOCAL FUNCTIONS
//******************************************************************************

#if defined(OMAPRPC_RUNTIME_DEBUG)
#warning "Building OMAPRPC with RUNTIME Debug!"
static uint32_t string_to_mask(char *string)
{
    uint32_t mask = 0;
    sscanf(string, "%x", &mask);
    return mask;
}

static void set_zone_mask()
{
    char *zone_string = getenv("OMAPRPC_ZONE_MASK");
    if (zone_string)
    {
        printf("Environment String: %s\n",zone_string);
        zone_mask = string_to_mask(zone_string);
    }
    else
    {
        zone_mask = SET_FLAG(OMAPRPC_ZONE_ERROR)|SET_FLAG(OMAPRPC_ZONE_PERF);
    }
    printf("OMAPRPC_ZONE_MASK=0x%x\n", OMAPRPC_ZONE_MASK);
    OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "ERRORS ARE ENABLED\n");
}
#endif

#if defined(OMAPRPC_USE_ION)
static int list_hdl_compare(node_t *a, node_t *b)
{
    struct ion_fd_data *ad = (struct ion_fd_data *)a->data;
    struct ion_fd_data *bd = (struct ion_fd_data *)b->data;
    if (ad->handle > bd->handle)
        return 1;
    else if (ad->handle < bd->handle)
        return -1;
    else
        return 0;
}
#endif

/*! \brief A private function used to restart the remote endpoint when the conditions
 * dictate that this should be done.
 * \param [in] rpc The handle to the OMAPRPC context.
 * \ingroup group_omaprpc
 */
static void omaprpc_restart(omaprpc_t *rpc)
{
    if (rpc && rpc->restart)
    {
        uint32_t retries = rpc->retries;

        // close the current file handle
        close(rpc->device);

        // we will try at least once.
        if (retries == 0)
            retries = 1;

        do {
            int ret = 0;
#if defined(POSIX)
            struct timespec req, rem;
            req.tv_sec = rem.tv_sec = rem.tv_nsec = 0;
            req.tv_nsec = 500000; // 5 ms (remote core reboots can take on the order of 20ms)
#endif
            // open a new handle
            rpc->device = open(rpc->dev_name, O_RDWR);
#if defined(POSIX)
            do {
                ret = nanosleep(&req, &rem);
                if (ret == -1 && errno == EINTR)
                {
                    OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Signal interrupted nanosleep.\n");
                    req.tv_nsec = rem.tv_nsec;
                }
            } while (ret != 0);
#endif
        } while (rpc->device <= 0 && --retries > 0);

        if (rpc->device > 0)
        {
            OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Re-opened device %s\n", rpc->dev_name);
            if (ioctl(rpc->device, OMAPRPC_IOC_CREATE, &rpc->create) < 0)
            {
                OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Failed to connect to remote service %s!\n", rpc->create.name);
                omaprpc_close(&rpc);
            }
            else
            {
                OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Connected to Service %s\n", rpc->create.name);

                // call the restart function
                rpc->in_restart = true_e;
                OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Calling Restart Function!\n");
                rpc->restart(rpc, rpc->cookie);
                rpc->in_restart = false_e;
            }
        }
    }
    else
    {
        OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Restart path not taken!\n");
    }
}

//******************************************************************************
// GLOBAL FUNCTIONS
//******************************************************************************

void omaprpc_close(omaprpc_t **prpc)
{
    if (prpc)
    {
        omaprpc_t *rpc = *prpc;
        if (rpc /*&& rpc->num_funcs > 0*/) // need to check something as non-zero.
        {
            ioctl(rpc->device, OMAPRPC_IOC_DESTROY, 0);
            close(rpc->device);
            OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Closed OMAPRPC Device!\n");
            rpc->device = -1; // set to invalid handle
            free(rpc);
            *prpc = NULL;
        }
    }
}

omaprpc_t *omaprpc_open(char *device_name, char *server_name, uint32_t numFuncs)
{
    omaprpc_t *rpc = NULL;

#if defined(OMAPRPC_RUNTIME_DEBUG)
    set_zone_mask();
    printf("OMAPRPC zone_mask=%x, error=%x is error masked?=%x\n", zone_mask, SET_FLAG(OMAPRPC_ZONE_ERROR), (SET_FLAG(OMAPRPC_ZONE_ERROR) & zone_mask));
#endif

    OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Opening OMAPRPC Device %s server: %s\n", device_name, server_name);
    rpc = (omaprpc_t *)calloc(1, sizeof(omaprpc_t));
    if (rpc)
    {
        strncpy(rpc->dev_name, device_name, MAX_PATH);

        rpc->device = open(device_name, O_RDWR);
        if (rpc->device < 0)
        {
            OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Failed to open device %s\n", device_name);
            omaprpc_close(&rpc);
        }
        else
        {
            OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Opened %s as FD:%u\n", device_name, rpc->device);

            memset(&rpc->create, 0, sizeof(rpc->create));
            strncpy(rpc->create.name, server_name, sizeof(rpc->create.name));
            OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Connecting to Service %s\n", rpc->create.name);
            if (ioctl(rpc->device, OMAPRPC_IOC_CREATE, &rpc->create) < 0)
            {
                OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Failed to connect to remote service %s!\n", rpc->create.name);
                omaprpc_close(&rpc);
            }
            else
            {
                OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Opened %s for %s\n", rpc->dev_name, rpc->create.name);
                rpc->num_funcs = numFuncs;
            }
        }
    }
    return rpc;
}

void omaprpc_restart_callback(omaprpc_t *rpc, void *cookie, omaprpc_endpoint_restarted_f func, uint32_t retries)
{
    if (rpc && func) // don't check cookie, it's ok if it is NULL;
    {
        OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Registered Restart Callback %p(%p)\n", func, cookie);
        rpc->restart = func;
        rpc->cookie = cookie;
        rpc->retries = retries;
    }
}

bool_e omaprpc_unregister(omaprpc_t *rpc, int memdevice, void *ptr, void **reserved)
{
    bool_e unregistered = false_e;
    if (rpc)
    {
#if defined(OMAPRPC_USE_ION)
        struct ion_fd_data data;
        node_t node, *tmp;
        memdevice = memdevice; // removes warning
        ptr = ptr; // removes warning
        data.handle = *((struct ion_handle **)reserved);
        if (ioctl(rpc->device, OMAPRPC_IOC_IONUNREGISTER, &data) >= 0)
            unregistered = true_e;
        if (unregistered) {
            OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Unregistered %p with OMAPRPC:%u\n", ptr, rpc->device);
        } else {
            OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Failed to unregister %p with OMAPRPC:%u\n", ptr, rpc->device);
        }

        node.data = (value_t)&data;
        tmp = list_remove_match(&rpc->fd_list, &node, list_hdl_compare);
        if (tmp)
        {
            struct ion_fd_data *fd_hdl = (struct ion_fd_data *)tmp->data;
            close(fd_hdl->fd);
            free(fd_hdl);
            free(tmp);
        }
#endif
    }
    return unregistered;
}

bool_e omaprpc_register(omaprpc_t *rpc, int memdevice, void *ptr, void **reserved)
{
    bool_e registered = false_e;
    if (rpc && memdevice && ptr && reserved)
    {
#if defined(OMAPRPC_USE_ION)
        struct ion_fd_data *data = calloc(1, sizeof(struct ion_fd_data));
        if (data)
        {
            struct ion_handle *ih = *((struct ion_handle **)reserved);
            int ret = ion_share(memdevice, ih, &data->fd);
            if (ret < 0)
            {
                OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Failed to share ION memory! (err=%d)\n", ret);
                free(data);
            }
            else
            {
                ret = ioctl(rpc->device, OMAPRPC_IOC_IONREGISTER, data);
                if (ret < 0)
                {
                    OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Failed to register ION buffer with OMAPRPC:%u! (err=%d)\n", rpc->device, ret);
                    close(data->fd);
                    free(data);
                }
                else
                {
                    node_t *node = node_create((value_t)data);
                    if (node)
                    {
                        list_append(&rpc->fd_list, node);
                    }
                    else
                    {
                        OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Failed to remember shared fd. LEAK!\n");
                    }

                    OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Registered %p with OMAPRPC:%u!\n", data->handle, rpc->device);
                    if (data->handle != ih) {
                        OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "ERROR: Handle from registration has changed! Was %p now %p\n", ih, data->handle);
                    }
                    *((struct ion_handle **)reserved) = data->handle;
                    registered = true_e;
                }
            }
        }
        else
        {
            OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "ERROR: Failed to create association structure!\n");
        }
#endif
    }
    return registered;
}


bool_e omaprpc_call(omaprpc_t *rpc, struct omaprpc_call_function_t *function, struct omaprpc_function_return_t *returned)
{
    clock_t start = 0, end = 0, diff = 0;
    bool_e passed = false_e;
    int ret = 0;
    uint32_t p, t;
    if (rpc && function->func_index < rpc->num_funcs)
    {
        size_t func_len = sizeof(struct omaprpc_call_function_t) + (function->num_translations * sizeof(struct omaprpc_param_translation_t));

        OMAPRPC_PRINT(OMAPRPC_ZONE_API, "Calling function %u\n", function->func_index);
        for (p = 0; p < function->num_params; p++)
        {
            switch (function->params[p].type)
            {
                case OMAPRPC_PARAM_TYPE_ATOMIC:
                    OMAPRPC_PRINT(OMAPRPC_ZONE_API, "ATM: param[%u] = %zu (size = %zu) resv=%zu",
                        p,
                        function->params[p].data,
                        function->params[p].size,
                        function->params[p].reserved);
                    break;
                case OMAPRPC_PARAM_TYPE_PTR:
                    OMAPRPC_PRINT(OMAPRPC_ZONE_API, "PTR: param[%u] = %p base = %p size = %zu resv = %zu",
                        p,
                        function->params[p].data,
                        function->params[p].base,
                        function->params[p].size,
                        function->params[p].reserved);
                    break;
                default:
                    break;
            }
        }
        for (t = 0; t < function->num_translations; t++)
        {
            OMAPRPC_PRINT(OMAPRPC_ZONE_API, "trans[%u] for param[%u]\n",
                t, function->translations[t].index)
        }

        start = clock();
        errno = 0; // reset errno before calling
        ret = write(rpc->device, function, func_len);
        end = clock();
        diff = end - start;
        OMAPRPC_PRINT(OMAPRPC_ZONE_PERF, "Called function %u, (wrote to fd %d took %lu ticks CLOCKS_PER_SEC=%lu)\n", function->func_index, rpc->device, diff, (clock_t)CLOCKS_PER_SEC);
        if (ret < 0)
        {
            passed = false_e;
            OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Failed to write to driver (ret=%d, errno=%d restart?=%s)\n", ret, errno, (rpc->in_restart==true_e?"yes":"no"));
            if (OMAPRPC_RESTART_CONDITION(ret) && rpc->in_restart == false_e)
            {
                OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Restart condition detected on write()!\n");
                omaprpc_restart(rpc);
            }
        }
        else
        {
            passed = true_e; // this could get superceded by read();
            if (returned) // provided pointer to get return value...
            {
                start = clock();
                errno = 0; // reset errno before calling.
                ret = read(rpc->device, returned, sizeof(*returned));
                end = clock();
                diff = end - start;
                OMAPRPC_PRINT(OMAPRPC_ZONE_PERF, "Returning Function %u (read on device fd %u took %lu ticks)\n", returned->func_index, rpc->device, diff);
                if (ret < 0)
                {
                    passed = false_e;
                    OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Failed to read from driver (ret=%d errno=%d restart?=%s)\n", ret, errno, (rpc->in_restart==true_e?"yes":"no"));
                    if (OMAPRPC_RESTART_CONDITION(ret) && rpc->in_restart == false_e)
                    {
                        OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Restart condition detected on read()!\n");
                        omaprpc_restart(rpc);
                    }
                }
                else
                {
                    passed = true_e;
                    OMAPRPC_PRINT(OMAPRPC_ZONE_INFO, "Function %u return value %d\n", returned->func_index, returned->status);
                }
            }
        }
    }
    else
    {
        OMAPRPC_PRINT(OMAPRPC_ZONE_ERROR, "Invalid parameters to call! rpc=%p, fi=%u, #funcs=%u\n", rpc, function->func_index, rpc->num_funcs);
    }
    OMAPRPC_PRINT(OMAPRPC_ZONE_API, "omaprpc_call(%p, %p, %p) => %s (errno=%d)\n",
            rpc, function, returned, (passed==true_e?"true":"false"), errno);
    return passed;
}
