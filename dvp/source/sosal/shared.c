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

#include <sosal/shared.h>
#include <sosal/debug.h>

#if defined(ANDROID)
#error "ANDROID must use the shared.cpp file"
#endif

void shared_free(shared_t **pshm)
{
    if (pshm)
    {
        shared_t *shm = *pshm;
        if (shm)
        {
#ifdef SHM_SYSVIPC
            shmdt(shm->data);
            shmctl(shm->shmid, IPC_RMID, 0);
#endif
#ifdef SHM_QNX
            munmap(shm->data, shm->size);
            close(shm->fd);
            shm_unlink(shm->name);
#endif
#ifdef WIN32
            UnmapViewOfFile(shm->data);
            CloseHandle(shm->mapfile);
#endif
            free(shm);
        }
        *pshm = NULL;
    }
}

shared_t *shared_alloc(unique_t un, size_t size)
{
    shared_t *shm = (shared_t *)calloc(1, sizeof(shared_t));
    if (shm)
    {
        shm->un = un;
        shm->size = size;
#ifdef SHM_SYSVIPC
        shm->shmflags = IPC_CREAT | 0666;
        shm->shmid = shmget(shm->un, shm->size, shm->shmflags);
        if (shm->shmid > -1)
        {
            SOSAL_PRINT(SOSAL_ZONE_SHARED, "SHM: seg=%d flags=%x\n", shm->shmid, shm->shmflags);
            shm->data = shmat(shm->shmid, NULL, 0);
            if (shm->data == (void *)-1)
            {
                shared_free(&shm);
            }
        }
#endif
#ifdef SHM_QNX
        sprintf(shm->name, "shared_mem_%x", shm->un);
        shm->fd = shm_open(shm->name, O_RDWR | O_CREAT, 0777);
        if (shm->fd > -1)
        {
            if (ftruncate(shm->fd, shm->size) > -1)
            {
                shm->data = mmap(0, shm->size, PROT_READ|PROT_WRITE, MAP_SHARED, shm->fd, 0);
                if (shm->data != MAP_FAILED)
                {
                    SOSAL_PRINT(SOSAL_ZONE_SHARED, "SHM: %s => %p for "FMT_SIZE_T" bytes\n", shm->name, shm->data, shm->size);
                }
                // if you clear the memory and someone else already has initialized it, you'll destroy data!
                //memset(shm->data, 0, sizeof(shm->data));
            }
        }
#endif
#ifdef WIN32
        wsprintf(shm->name, TEXT("shared_mem_%x\n"), shm->un);
        shm->mapfile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, shm->name);
        if (shm->mapfile)
        {
            shm->data = MapViewOfFile(shm->mapfile, FILE_MAP_ALL_ACCESS, 0, 0, shm->size);
            if (shm->data == NULL)
            {
                shared_free(&shm);
            }
        }
#endif
        if (shm)
        {
            SOSAL_PRINT(SOSAL_ZONE_SHARED, "SHM[%x]: Size="FMT_SIZE_T" Data=%p\n", shm->un, shm->size, shm->data);
        }
    }
    return shm;
}


#include <sosal/thread.h>
bool_e shared_unittest(int argc, char *argv[])
{
    int s = (argc > 1 ? atoi(argv[1]) : 7000);
    unique_t un = (argc > 2 ? atoi(argv[2]) : 42);
    char string[MAX_PATH];
    shared_t *shm = shared_alloc(un, MAX_PATH);
    if (shm)
    {
        memset(string, 0, MAX_PATH);
        if (argc > 3) {
            SOSAL_PRINT(SOSAL_ZONE_SHARED, "Copying %s into %p\n", argv[3], shm->data);
            strncpy((char *)shm->data, argv[3], MAX_PATH);
        }
        thread_msleep(s);
        strncpy(string, (char *)shm->data, MAX_PATH);
        SOSAL_PRINT(SOSAL_ZONE_SHARED, "String = %s\n", string);

        shared_free(&shm);
    }
    SOSAL_PRINT(SOSAL_ZONE_SHARED, "SHM: Exiting\n");
    return true_e;
}

