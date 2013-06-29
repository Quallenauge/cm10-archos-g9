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

#include <sosal/mutex.h>
#include <sosal/debug.h>

void mutex_init(mutex_t *m)
{
#ifdef POSIX
    pthread_mutex_init(m, NULL);
#elif defined(SYSBIOS) 
    /* m already allocated */
    m->key = NULL;
    /*Creating a gate with default params (NULL)*/
    /* No calling of  GateMutexPri_Params_init(&params); to prepare them */
    m->gate = GateMutexPri_create(NULL, NULL);
#else
    InitializeCriticalSection(m);
#endif
}

void mutex_deinit(mutex_t *m)
{
#ifdef POSIX
    pthread_mutex_destroy(m);
#elif defined(SYSBIOS)
    /* Delete the mutex.  */
    GateMutexPri_delete(&(m->gate));
    /* deallocation on the calling level */
#else
    DeleteCriticalSection(m);
#endif
}

void mutex_lock(mutex_t *m)
{
#ifdef POSIX
    pthread_mutex_lock(m);
#elif defined(SYSBIOS)
    m->key = GateMutexPri_enter(m->gate);
#else
    EnterCriticalSection(m);
#endif
}

void mutex_unlock(mutex_t *m)
{
#ifdef POSIX
    pthread_mutex_unlock(m);
#elif defined(SYSBIOS)
    /* Release the mutex.  */
    GateMutexPri_leave(m->gate, m->key);
#else
    LeaveCriticalSection(m);
#endif
}

