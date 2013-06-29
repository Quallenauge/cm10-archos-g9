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

#ifdef SYSBIOS
#include <sosal/mutex.h>
#endif
#include <sosal/event.h>
#include <sosal/debug.h>

#if defined(POSIX) && !defined(ESUCCESS)
#define ESUCCESS 0
#endif

#ifdef POSIX
void milliseconds_from_now(struct timespec *time_spec, uint32_t milliseconds)
{
    struct timeval now;

    gettimeofday(&now, NULL);
    time_spec->tv_sec = now.tv_sec + (milliseconds / 1000);
    time_spec->tv_nsec = (now.tv_usec * 1000) + ((milliseconds%1000) * 1000000);
    if (time_spec->tv_nsec > 1000000000) {
        time_spec->tv_sec += 1;
        time_spec->tv_nsec -= 1000000000;
    }
}
#endif

bool_e event_init(event_t *e, bool_e autoreset)
{
#ifdef POSIX
    int err = 0;
    err |= pthread_mutex_init(&e->mutex, NULL);
    err |= pthread_condattr_init(&e->attr);
    err |= pthread_cond_init(&e->cond, &e->attr);
    e->set = false_e;
    e->autoreset = autoreset;
    if (err == ESUCCESS)
#elif defined(SYSBIOS)
    mutex_init(&e->mutex);
    e->event_group = Event_create(NULL, NULL);
    e->set = false_e;
    e->autoreset = autoreset;
    if(e->event_group != NULL)
#else
    BOOL bManual = TRUE;
    if (autoreset)
        bManual = FALSE;
    *e = CreateEvent(NULL, bManual, FALSE, NULL);
    if (*e != NULL)
#endif
        return true_e;
    else
        return false_e;
}

bool_e event_deinit(event_t *e)
{
#ifdef POSIX
    int err = 0;
    do {
        err = pthread_cond_destroy(&e->cond);
        if (err == EBUSY) {
            pthread_mutex_lock(&e->mutex);
            e->set = false_e;
            pthread_cond_broadcast(&e->cond);
            pthread_mutex_unlock(&e->mutex);
        }
    } while (err != 0);
    pthread_condattr_destroy(&e->attr);
    pthread_mutex_destroy(&e->mutex);
    return true_e;
#elif defined(SYSBIOS)
    mutex_deinit(&e->mutex);
    Event_delete(&e->event_group);
    return true_e;
#else
    return CloseHandle(*e);
#endif
}

#ifdef POSIX
static bool_e event_timed_wait(event_t *e, uint32_t ms)
{
    int retcode = 0;
    bool_e ret = false_e;
    if (ms < EVENT_FOREVER)
    {
        struct timespec time_spec;
        milliseconds_from_now(&time_spec, ms);
        //SOSAL_PRINT(SOSAL_ZONE_EVENT, "Waiting for at least %u ms or until %lu:%lu\n", ms, time_spec.tv_sec, time_spec.tv_nsec);
        retcode = pthread_cond_timedwait(&e->cond, &e->mutex, &time_spec);
    }
    else
    {
        //SOSAL_PRINT(SOSAL_ZONE_EVENT, "Waiting indefinitely for event!\n");
        retcode = pthread_cond_wait(&e->cond, &e->mutex);
    }
    if (retcode == ETIMEDOUT && e->set == false_e) {
        SOSAL_PRINT(SOSAL_ZONE_WARNING, "WARNING: Event %p Timeout!\n", e);
        ret = false_e;
    } else if (retcode == ETIMEDOUT && e->set == true_e) {
        // this should not have happened (but the signal is set, so we need to return true).
        ret = true_e;
    } else if (retcode == ESUCCESS && e->set == false_e) {
        // the condition was signalled but someone has either lowered or never set the flag.
        SOSAL_PRINT(SOSAL_ZONE_WARNING, "WARNING: Kicking out of event %p!\n", e);
        ret = false_e;
    } else if (retcode == ESUCCESS && e->set == true_e) {
        // someone called "set"
        //SOSAL_PRINT(SOSAL_ZONE_EVENT, "SOSAL: Event %p set!\n", e);
        ret = true_e;
    } else {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR: General Error in Event %p code %d\n", e, retcode);
    }
    return ret;
}
#endif

bool_e event_wait(event_t *e, uint32_t timeout)
{
    bool_e ret = false_e;
#ifdef POSIX
    pthread_mutex_lock(&e->mutex);
    SOSAL_PRINT(SOSAL_ZONE_EVENT, "SOSAL: Waiting on Event %p (%s) for %u ms\n", e, ((e->autoreset)?"Auto":"Manual"), timeout);
    if (!e->autoreset)
    {
        if (e->set == false_e)
            ret = event_timed_wait(e, timeout);
        else
            ret = true_e;
    }
    else // "pulse" mode events
    {
        ret = event_timed_wait(e, timeout);
        // if ret is false_e then timeout
        // if ret is true_e and e->set is true_e then signalled (reset the event)
        // if ret is true_e and e->set is false_e then we're being kicked out
        if (ret == true_e && e->set == true_e)
            e->set = false_e;
        else if (ret == true_e && e->set == false_e)
            ret = false_e;
    }
    SOSAL_PRINT(SOSAL_ZONE_EVENT, "SOSAL: Event %p ret = %u set = %u!\n", e, ret, e->set);
    pthread_mutex_unlock(&e->mutex);
    return ret;
#elif defined(SYSBIOS)
    unsigned int andMask, orMask;
    mutex_lock(&e->mutex);
    SOSAL_PRINT(SOSAL_ZONE_EVENT, "SOSAL: Waiting on Event %p (%s) for %u ms\n", e, ((e->autoreset)?"Auto":"Manual"), timeout);
    andMask = SYSBIOS_SINGLE_EVENT_FLAG_MASK;
    orMask = 0;
    if (!e->autoreset)
    {
        if (e->set == false_e)
            ret = (bool_e)Event_pend(e->event_group, andMask, orMask, timeout);
        else
            ret = true_e;
    }
    else
    {
        ret = (bool_e)Event_pend(e->event_group, andMask, orMask, timeout);
        // Event_pend() returns number of consumed events or zero if timeout
        // if ret is false_e then timeout
        // if ret is true_e and e->set is true_e then signalled (reset the event)
        // if ret is true_e and e->set is false_e then we're being kicked out
        if (ret == true_e && e->set == true_e)
            e->set = false_e;
        else if (ret == true_e && e->set == false_e)
            ret = false_e;
    }
    SOSAL_PRINT(SOSAL_ZONE_EVENT, "SOSAL: Event %p ret = %u set = %u!\n", e, ret, e->set);
    mutex_unlock(&e->mutex);
#else
    DWORD status = WaitForSingleObject(*e, timeout);
    if (status == WAIT_OBJECT_0)
        ret = true_e;
#endif
    return ret;
}

bool_e event_set(event_t *e)
{
#ifdef POSIX
    int err = 0;
    pthread_mutex_lock(&e->mutex);
    //SOSAL_PRINT(SOSAL_ZONE_EVENT, "Setting Event %p\n", e);
    e->set = true_e;
    err = pthread_cond_broadcast(&e->cond);
    pthread_mutex_unlock(&e->mutex);
    if (err == ESUCCESS)
        return true_e;
    else
        return false_e;
#elif defined(SYSBIOS)
    //mutex_lock(&e->mutex);
    e->set = true_e;
    Event_post(e->event_group, SYSBIOS_SINGLE_EVENT_FLAG_MASK);
    //mutex_unlock(&e->mutex);
    return true_e;
#else
    return (bool_e)SetEvent(*e);
#endif
}

bool_e event_reset(event_t *e)
{
#ifdef POSIX
    pthread_mutex_lock(&e->mutex);
    //SOSAL_PRINT(SOSAL_ZONE_EVENT, "Resetting Event %p\n", e);
    e->set = false_e;
    pthread_mutex_unlock(&e->mutex);
    return true_e;
#elif defined(SYSBIOS)
    mutex_lock(&e->mutex);
    e->set = false_e;
    mutex_unlock(&e->mutex);
    return true_e;
#else
    return (bool_e)ResetEvent(*e);
#endif
}

bool_e event_unittest(int argc __attribute__((unused)),
                      char *argv[] __attribute__((unused)))
{
    event_t e;
    int numErrors = 0;
    uint32_t timeout = 100;

    SOSAL_PRINT(SOSAL_ZONE_API, "event_unittest(%u, %p)\n", argc, argv);

    event_init(&e, false_e);

    // wait on the event, should fail as not signaled.
    if (event_wait(&e, timeout) == true_e)
    {
        numErrors++;
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Failed to wait for timeout!\n");
    }

    // signal the event
    event_set(&e);

    // wait on the event forever, should succeed
    if (event_wait(&e, EVENT_FOREVER) == false_e)
    {
        numErrors++;
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Failed to wait for signal!\n");
    }

    // lower the event
    event_reset(&e);

    // wait again, should fail
    if (event_wait(&e, timeout) == true_e)
    {
        numErrors++;
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Failed to wait for timeout 2!\n");
    }

    event_deinit(&e);

    if (numErrors > 0)
        return false_e;
    else
        return true_e;
}

