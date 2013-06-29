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

#ifndef _SOSAL_EVENT_H_
#define _SOSAL_EVENT_H_

/*!
 * \file
 * \brief A simple event mechanism.
 * \author Erik Rainey
 */

#include <sosal/types.h>

#ifdef POSIX
#if !defined(__timespec_defined) && !defined(ANDROID) && !defined(__QNX__) && !defined(CYGWIN) && !defined(DARWIN)
/*! Redefinition of the native timespec structure */
struct timespec
{
    __time_t tv_sec;        /*!< Seconds.  */
    long int tv_nsec;       /*!< Nanoseconds.  */
};
#endif

/*!
 * \brief The SOSAL Event Data Type.
 * In a POSIX environment, this wraps the pthread implementations of conditions and a mutex.
 * \ingroup group_events
 */
struct _event_posix_t {
    pthread_cond_t     cond;        /*!< The PThread Condition */
    pthread_condattr_t attr;        /*!< The PThread Condition Attribute */
    pthread_mutex_t    mutex;       /*!< The PThread Mutex */
    bool_e             autoreset;   /*!< Indicates whether the event will auto-reset after signalling */
    bool_e             set;         /*!< The current event value */
};

/*! \brief The SOSAL Event Type.
 * \ingroup group_events
 */
typedef struct _event_posix_t event_t;

#elif defined(SYSBIOS)

#include <ti/sysbios/knl/Event.h>

#define SYSBIOS_SINGLE_EVENT_FLAG_MASK (1<<0)
#include <sosal/mutex.h>
struct _event_sysbios_t {
    mutex_t            mutex;       /*!< The PThread Mutex */
    bool_e             autoreset;   /*!< Indicates whether the event will auto-reset after signalling */
    bool_e             set;         /*!< The current event value */
    Event_Handle       event_group;
};

typedef struct _event_sysbios_t event_t;

#else

/*! \brief In Windows this wraps the a HANDLE which is used in the Event API.
 * \ingroup group_events
 */
typedef HANDLE event_t;

#endif

/*! \brief The longest time possible for an event to wait.
 * \ingroup group_events
 */
#define EVENT_FOREVER   (0xFFFFFFFF)

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Initializes an event.
 * \param [in] e The pointer to the event structure
 * \param [in] autoreset Determine whether the event autoresets
 * \return bool_e
 * \ingroup group_events
 */
bool_e event_init(event_t *e, bool_e autoreset);

/*!
 * \brief Deinitializes an event.
 * \param [in] e The pointer to the event structure
 * \ingroup group_events
 */
bool_e event_deinit(event_t *e);

/*! \brief Waits for an event to trigger for a timeout period.
 * \param [in] e The pointer to the event.
 * \param [in] timeout The period in milliseconds to wait for the timeout.
 * \retval false_e The wait timed-out.
 * \retval true_e The event triggered.
 * \ingroup group_events
 */
bool_e event_wait(event_t *e, uint32_t timeout);

/*! \brief Triggers the event. If the event is auto-resetting, the state will be reset
 * once the function returns. If not, a call to event_reset is required.
 * \param [in] e The pointer to the event.
 * \ingroup group_events
 */
bool_e event_set(event_t *e);

/*! \brief Resets an event to un-triggered.
 * \param [in] e The pointer to the event.
 * \ingroup group_events
 */
bool_e event_reset(event_t *e);

// INTERNAL FUNCTIONS
typedef struct timespec tspec;
void milliseconds_from_now(tspec *time_spec, uint32_t milliseconds);


#ifdef __cplusplus
}
#endif

#endif

