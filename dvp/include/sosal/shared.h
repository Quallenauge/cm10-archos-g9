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

#ifndef _SOSAL_SHARED_H_
#define _SOSAL_SHARED_H_

/*! \file  
 * \brief The SOSAL Shared Memory API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */
#include <sosal/types.h>

#if defined(ANDROID) && (defined(FROYO) || defined(GINGERBREAD) || defined(ICS))
#undef SHM_SYSVIPC
#undef SHM_QNX
#define SHM_ASHMEM
// Use Asynchronous Shared Memory
#include <cutils/ashmem.h>
#include <cutils/properties.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(ANDROID) // NDK
#undef SHM_SYSVIPC
#undef SHM_QNX
#define SHM_ASHMEM
#include <linux/ashmem.h>
#elif defined(LINUX) || defined(DARWIN)
#undef SHM_ASHMEM
#undef SHM_QNX
#define SHM_SYSVIPC
// Use SYS V Shared Memory API
#include <sys/ipc.h>
#include <sys/shm.h>
#elif  defined(__QNX__)
#undef SHM_ASHMEM
#undef SHM_SYSVIPC
#define SHM_QNX
#include <fcntl.h>
#include <sys/mman.h>
#elif defined(WIN32) || defined(CYGWIN)
#undef SHM_ASHMEM
#undef SHM_QNX
#undef SHM_SYSVIPC
// Use Shared Memory File API
#include <windows.h>
#endif

/*! \brief The method to identify each shared memory block from each other. 
 * \ingroup group_shared
 */
typedef int32_t unique_t;

/*! \brief The structure which maintains the information about the memory sharing.
 * \ingroup group_shared
 */
typedef struct _shared_t {
    unique_t un;    /*!< \brief The unique identifier of this block */
    size_t size;     /*!< \brief The size of the block in bytes. */
    void *data;      /*!< \brief The pointer to the data buffer */
#ifdef SHM_SYSVIPC
    int shmid;       /*!< \brief The shared memory identifier */
    int shmflags;    /*!< \brief The shared memory flags */
    key_t key;       /*!< \brief The key used to access the shared memory */
#endif
#if defined(SHM_ASHMEM) || defined(SHM_QNX)
    int fd;          /*!< \brief The file descriptor used to share the memory */
    void *reserved;
#endif
#if defined(WIN32) || defined(CYGWIN)
    HANDLE mapfile; /*!< \brief The file handle to the MAPPED memory */
    TCHAR name[MAX_PATH]; /*!< \brief The name of the shared data */
#endif
} shared_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This function allocates shared memory. If the unique identifier 
 * already exists, the memory is mapped into your memory space.
 * \param [in] un The unique identifier. 
 * \param [in] size The bytes size of the memory aread needed.
 * \return Returns the shared_t memory handle.
 * \retval NULL if the shared memory failed to allocate.
 * \post \ref shared_free
 * \ingroup group_shared
 */
shared_t *shared_alloc(unique_t un, size_t size);

/*! \brief This function frees shared memory. If there are multiple clients
 * still open after this free is called, the memory will not be actually freed. 
 * \param [in] pshm The pointer to the shared memory handle. 
 * \pre \ref shared_alloc
 * \ingroup group_shared
 */
void shared_free(shared_t **pshm);

#ifdef __cplusplus
}
#endif

#endif

