/**
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

#ifndef _SOSAL_THREADPOOL_H_
#define _SOSAL_THREADPOOL_H_

/*! \file
 * \brief Threadpools allow an abstraction around a series of N worker threads attached to 
 * a queue of work per thread. 
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>
#include <sosal/thread.h>
#include <sosal/queue.h>
#include <sosal/semaphore.h>
#include <sosal/profiler.h>

// forward typedef
struct _threadpool_worker_t; 

/*! \brief The function pointer to the worker function. 
 * \param [in] worker The per-thread worker data structure.
 * \retval false_e Indicates that the worker failed to process data or had some other
 * error.
 * \ingroup group_threadpools
 */
typedef bool_e (*threadpool_f)(struct _threadpool_worker_t *worker);

/*! \brief The structure given to each threadpool worker during execution.
 * \ingroup group_threadpools
 */
typedef struct _threadpool_worker_t {
    queue_t *queue;				/*!< \brief The work queue */
    thread_t handle;			/*!< \brief The handle to the worker thread */
    uint32_t index;				/*!< \brief The index of this worker in the pool */
    bool_e   active;			/*!< \brief Indicates whether this worker is currently operating. */
    threadpool_f function;		/*!< \brief The worker function */
    void    *arg;				/*!< \brief The user argument to the thread */
    void    *data;				/*!< \brief The user data pointer */
    struct _threadpool_t *pool; /*!< \brief Pointer to the top level structure. */
    profiler_t perf;			/*!< \brief Performance capture variable. */
} threadpool_worker_t;

/*! \brief The internal structure for tracking a threadpool.
 * \ingroup group_threadpools
 */
typedef struct _threadpool_t {
    uint32_t numWorkers;			/*!< \brief The number of threads in the pool */
    uint32_t numWorkItems;			/*!< \brief The maximum number of threads in the queue */
    uint32_t sizeWorkItem;			/*!< \brief Unit size of a work item */
     int32_t numCurrentItems;		/*!< \brief The number of corrent items in the queue */
    threadpool_worker_t *workers;	/*!< \brief The array of workers */
    uint32_t nextWorkerIndex;		/*!< \brief The next index to submit work to */
    semaphore_t sem;				/*!< \brief The semaphore which protect access to the work queues */
    event_t  completed;				/*!< \brief The event which indicates that all work is completed */
} threadpool_t;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief This function destroys a threadpool. It will NOT wait for completion.
 * \param [in] pool The pointer to the threadpool_t returned by \ref threadpool_create.
 * \pre \ref threadpool_create
 * \note The user must set the pointer to NULL after this function.
 * \ingroup group_threadpools
 */
void threadpool_destroy(threadpool_t *pool);

/*!
 * \brief This function creates a threadpool for a supplied number of threads.
 * \param [in] numThreads How many threads to distribute the work around to.
 * \param [in] numWorkItems How deep to make the work queues.
 * \param [in] sizeWorkItem How large each work item is.
 * \param [in] worker The function which implements the workers.
 * \param [in] arg A user supplied global memory pointer for each thread.
 * \ingroup group_threadpools
 */
threadpool_t *threadpool_create(uint32_t numThreads, uint32_t numWorkItems, size_t sizeWorkItem, threadpool_f worker, void *arg);

/*!
 * \brief This function issues work to the threapool to be completed. This will return when the
 * work has been put into the queues but not necessarily when the work is
 * complete.
 * \param [in] pool The pointer to the threadpool_t returned by \ref threadpool_create.
 * \param [in] workitems An array of work pointers.
 * \param [in] numWorkItems A count of the number of work pointers in the workitems array.
 * \ingroup group_threadpools
 * \pre \ref threadpool_create
 * \post \ref threadpool_complete
 */
bool_e threadpool_issue(threadpool_t *pool, void *workitems[], uint32_t numWorkItems);

/**
 * \brief This function will get the status of the threadpool work queues. If set
 * to blocking, this will return once all workers have completed.
 * \param [in] pool The pointer to the threadpool_t returned by \ref threadpool_create.
 * \param [in] blocking Set to true_e for synchronous and false_e for the
 * current status (polling mode).
 * \ingroup group_threadpools
 * \retval true_e Work has completed
 * \retval false_e Work has not yet completed.
 */
bool_e threadpool_complete(threadpool_t *pool, bool_e blocking);

#ifdef __cplusplus
}
#endif

#endif
