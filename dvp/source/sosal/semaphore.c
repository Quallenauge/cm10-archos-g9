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

#include <sosal/semaphore.h>
#include <sosal/debug.h>

bool_e semaphore_create(semaphore_t *s, uint32_t count, bool_e shared)
{
#ifdef POSIX
    if (sem_init(s, (shared == true_e ? 1 : 0), count) == 0)
#elif defined(SYSBIOS)
    /* creating with default params (NULL) */
    *s = Semaphore_create(count, NULL, NULL);
    if (*s != NULL)
#else
    *s = CreateSemaphore(NULL, count, count, NULL);
    if (*s != NULL)
#endif
        return true_e;
    else
        return false_e;
}

bool_e semaphore_delete(semaphore_t *s)
{
#ifdef POSIX
    if (sem_destroy(s) == 0)
        return true_e;
    else
        return false_e;
#elif defined(SYSBIOS)
    /* Delete the semaphore.  */
    Semaphore_delete(s);
    return true_e;
#else
    if (CloseHandle(*s) == TRUE)
        return true_e;
    else
        return false_e;
#endif
}

bool_e semaphore_post(semaphore_t *s)
{
#ifdef POSIX
    int ret = sem_post(s);
    SOSAL_PRINT(SOSAL_ZONE_SEMA, "SEM POST on %p returned %d, errno=%d\n", s, ret, errno);
    if (ret == 0)
        return true_e;
    else
        return false_e;
#elif defined(SYSBIOS)
    /* Release the semaphore.  */
    Semaphore_post(*s);
    return true_e;
#else
    if (ReleaseSemaphore(*s, 1, NULL) == TRUE)
        return true_e;
    else
        return false_e;
#endif
}

bool_e semaphore_wait(semaphore_t *s)
{
#ifdef POSIX
    int ret = sem_wait(s);
    SOSAL_PRINT(SOSAL_ZONE_SEMA, "SEM WAIT on %p returned %d, errno=%d\n", s, ret, errno);
    if (ret == 0)
#elif defined(SYSBIOS)
    int ret = 1;
    if(s != 0 && (*s) != 0) {
    	ret = Semaphore_pend(*s, BIOS_WAIT_FOREVER); //returns TRUE on success
    }
    if (ret != 0)
#else
    DWORD status = WaitForSingleObject(*s, INFINITE);
    if (status == WAIT_OBJECT_0)
#endif
        return true_e;
    else
        return false_e;
}

int semaphore_trywait(semaphore_t *s)
{
#ifdef POSIX
    int ret = sem_trywait(s);
    SOSAL_PRINT(SOSAL_ZONE_SEMA, "SEM TRYWAIT on %p returned %d, errno=%d\n", s, ret, errno);
    if (ret == 0)
        return 1;
    else if (ret == -1 && errno == EAGAIN)
        return 0;
    else
        return -1;
#elif defined(SYSBIOS)
    int ret = Semaphore_pend(*s, BIOS_NO_WAIT); //returns TRUE on success
    if( ret != 0 )
        return 1;
    else
        return -1;
#else
    DWORD status = WaitForSingleObject(*s, 0);
    if (status == WAIT_OBJECT_0)
        return 1;
    else if (status == WAIT_TIMEOUT)
        return -1;
    else
        return -1;
#endif
}

