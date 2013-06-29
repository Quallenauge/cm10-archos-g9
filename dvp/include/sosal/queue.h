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

#ifndef _SOSAL_QUEUE_H_
#define _SOSAL_QUEUE_H_

/*! \file
 * \brief The SOSAL Queues API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */
 
#include <sosal/types.h>
#include <sosal/mutex.h>
#include <sosal/event.h>
#include <sosal/ring.h>

/*! \brief The queue data structure
 * \see queue_create
 * A queue is a ring buffer with \ref mutex_t protection and two
 * \ref event_t to indicate read and write access.
 * \ingroup group_queues
 */
typedef struct _queue_t {
    bool_e  active;
    bool_e  popped;
    ring_t *ringb;
    size_t  msgSize;
    size_t  numMsgs;
    size_t  msgCount;
    mutex_t access;
    event_t readEvent;
    event_t writeEvent;
} queue_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Creates a queue with a default number of message of a set size. Once
 * created, this can not be changed.
 * \returns queue_t *
 * \retval NULL Could not create the queue.
 * \param [in] numMsgs The number of messages to hold in the ring buffer.
 * \param [in] msgSize The size in bytes of the message.
 * \ingroup group_queues
 */
queue_t *queue_create(size_t numMsgs, size_t msgSize);

/*! \brief Destroys a queue.
 * \note The queue will be popped, which removes all listeners, then
 * deleted, which will destroy all message in the queue.
 * \param q The queue to destroy.
 * \ingroup group_queues
 */
void queue_destroy(queue_t *q);

/*! \brief Allows the client to write data unto the queue. If blocking is enabled
 * the function will block until space is available in the queue.
 * \param q The queue to used.
 * \param blocking If true, blocking could occur if no space is available.
 * \param datum A pointer to a message to write into the ring buffer.
 * \returns bool_e
 * \retval true_e If data was successfully written.
 * \retval false_e If any error occurred.
 * \ingroup group_queues
 */
bool_e queue_write(queue_t *q, bool_e blocking, void *datum);

/*! \brief Allows a client to read data out of the queue. If blocking is enabled
 * the function will block until data is available to be read.
 * \param q The queue to used.
 * \param blocking If true, blocking could occur if no space is available.
 * \param datum A pointer to a location to write the message in the ring buffer to.
 * \returns bool_e
 * \retval true_e If data was successfully read.
 * \retval false_e If any error occurred (When not blocking it could indicate
 * that no data is ready).
 * \ingroup group_queues
 */
bool_e queue_read(queue_t *q, bool_e blocking, void *datum);

/*! \brief Enables read and write events on the queue. Waiting listeners may
 * get events depending on the data state of the queue.
 * \param q The queue to affect.
 * \ingroup group_queues
 */
void queue_enable(queue_t *q);

/*! \brief Disables read and write events on the queue. No listeners will be
 * signalled after this is called. You must re-enable later to get events
 * again.
 * \see queue_enable
 * \param q The queue to affect.
 * \ingroup group_queues
 */
void queue_disable(queue_t *q);

/*! \brief Forcibly sets the read and write events but causes \ref false_e to be
 * returned from read or write functions. This removes listeners and
 * preserves the data in the queue.
 * \param q The queue to affect.
 * \ingroup group_queues
 */
void queue_pop(queue_t *q);

/*! \brief Returns the queue to a normal state from \ref queue_pop. Listeners
 * will get appropriate events after this call. This call may cause some
 * listeners to signal.
 * \param q The queue to affect.
 * \ingroup group_queues
 */
void queue_unpop(queue_t *q);

/*! \brief Returns the number of used slots in the ring buffer.
 * \returns size_t The number of units in the ring buffer.
 * \param q The queue to query.
 * \ingroup group_queues
 */
size_t queue_length(queue_t *q);

/*! \brief Reallocates the internal ring buffer and resets the \ref event_t for
 * read and write access.
 * \param q The queue to affect.
 * \ingroup group_queues
 */
queue_t *queue_reset(queue_t *q);

#ifdef __cplusplus
}
#endif

#endif
