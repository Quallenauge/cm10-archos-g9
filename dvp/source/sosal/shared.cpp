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

#include <shared_binder.h>
using namespace android;

void shared_free(shared_t **pshm)
{
    sp<ISharedMemory> shm_service;
    status_t status = getService<ISharedMemory>(android::String16(SHARED_MEMORY_SERVICE_NAME), &shm_service);
    if (status == NO_ERROR) {
        shm_service->forget(pshm);
    } else {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to obtain memory service handle! (0x%x)", status);
    }
}

shared_t *shared_alloc(unique_t un, size_t size)
{
    shared_t *shm = (shared_t *)calloc(1, sizeof(shared_t));
    if (shm)
    {
        shm->un = un;
        shm->size = size;
        {
            sp<ISharedMemory> shm_service;
            status_t status = getService<ISharedMemory>(android::String16(SHARED_MEMORY_SERVICE_NAME), &shm_service);
            if (status == NO_ERROR)
            {
                if (shm_service->allocate(&shm) == true_e)
                {
                    SOSAL_PRINT(SOSAL_ZONE_SHARED, "SHM[%x]: FD=%d\n", shm->un, shm->fd);
                }
            }
            else
            {
                free(shm);
                shm = NULL;
            }
        }
        if (shm)
        {
            SOSAL_PRINT(SOSAL_ZONE_SHARED, "SHM[%x]: Size="FMT_SIZE_T" Data=%p\n", shm->un, shm->size, shm->data);
        }
    }
    return shm;
}


#include <sosal/thread.h>
extern "C" bool_e shared_unittest(int argc, char *argv[])
{
    bool_e ret = true_e;
    int s = (argc > 1 ? atoi(argv[1]) : 7000);
    unique_t un = (argc > 2 ? atoi(argv[2]) : 42);
    char string[MAX_PATH];
    shared_t *shm = shared_alloc(un, MAX_PATH);
    SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "SHM: Testing Running!\n");
    if (shm)
    {
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "SHM: %p FD=%d\n", shm, shm->fd);

        memset(string, 0, MAX_PATH);
        if (argc > 3) {
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Copying %s into %p\n", argv[3], shm->data);
            strncpy((char *)shm->data, argv[3], MAX_PATH);
        }
        thread_msleep(s);
        strncpy(string, (char *)shm->data, MAX_PATH);
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "String = %s\n", string);

        shared_free(&shm);
    }
    else
        ret = false_e;
    SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "SHM: Exiting\n");
    return ret;
}

