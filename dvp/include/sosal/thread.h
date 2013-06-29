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

#ifndef _SOSAL_THREAD_H_
#define _SOSAL_THREAD_H_

/*! \file
 * \brief The threading interface for SOSAL.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>

#ifdef POSIX
/*! \brief In POSIX environments, this is a typedef of the pthread construct.
 * \ingroup group_threads
 */
typedef pthread_t thread_t;

/*! \brief The return type for threads.
 * \ingroup group_threads 
 */
typedef void *    thread_ret_t;
#define THREAD_FMT     "%lu"
#define THREAD_RET_FMT "%p"

#elif defined(SYSBIOS)

#include <sosal/event.h>
#include <sosal/semaphore.h>
#include <ti/sysbios/knl/Task.h>
typedef  int     thread_ret_t;
#define SYSBIOS_DEFAULT_PRIORITY    (5)
/*
 * The default value of 0 means that the module config defaultStackSize is used.
 */
#define SYSBIOS_DEFAULT_STASK_SIZE  (0)  //(64 * 1024)
//#define JOIN_SEMAPHORE
#ifndef JOIN_SEMAPHORE
typedef struct _thread_ack_sysbios_t ack_t;
struct _thread_ack_sysbios_t {
    event_t*        exit_event_ack; /*!< Event used for join notification */
    ack_t*          next;
};
#endif

typedef thread_ret_t (*thread_f)(void *arg);
struct _thread_sysbios_t {
    Task_Handle     thread_hdl;     /*!< SYSBIOS thread handle */
#ifdef JOIN_SEMAPHORE
    semaphore_t*    join_semaphore; /*!< Semapfor used for join control */
    mutex_t*        join_mutex;     /*!< Mutex used for protection of join_number update */
    uint32_t        join_number;    /*!< Counter of join users */
#else
    event_t*        exit_event;     /*!< Event used for thread exit notification */
    ack_t*          pAckList;       /*!< List of clients waiting for thread join */
#endif
/*  void*           stack_ptr; */   /*!< Pointer to stack of SYSBIOS thread */
    void*           client_arg;     /*!< Client arguments for thread main func */
    thread_f        client_func;    /*!< Client thread main func */
    uint32_t        exit_code;      /*!< Client thread main func exit code*/
    bool_e          bTaskReady;     /*!< Set, when task main func is done */
};
typedef struct _thread_sysbios_t* thread_t;


#define THREAD_FMT     "%lu"
#define THREAD_RET_FMT "%d"

#elif defined(WIN32) || !defined(CYGWIN)

/*! \brief In Windows environments, this is a typedef of the Thread HANDLE.
 * \ingroup group_threads
 */
typedef HANDLE    thread_t;

/*! \brief The return type for threads. 
 * \ingroup group_threads
 */
typedef DWORD     thread_ret_t;
#define THREAD_FMT     "%lu"
#define THREAD_RET_FMT "%d"

#endif

#define assign_thread_ret(r, x, typex)  r = (thread_ret_t)((typex)(value_t)r + x)

#ifndef SYSBIOS
/*! \brief The thread function pointer.  
 * \ingroup group_threads
 */
typedef thread_ret_t (*thread_f)(void *arg);
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Creates a new thread which executes the provided function with the provided argument.
 * \ingroup group_threads
 */
thread_t thread_create(thread_f func, void *arg);

#ifdef POSIX

/**
 * \brief Causes a thread to return.
 * \ingroup group_threads
 */
void thread_return(thread_ret_t r);

/**
 * \brief Used to exit from a thread.
 * \ingroup group_threads
 */
#define thread_exit(r)  thread_return((thread_ret_t)r); return NULL

#else

/**
 * \brief Used to exit from a thread.
 * \ingroup group_threads
 */
#define thread_exit(r)  return (r)

#endif

/**
 * \brief Used to wait for the provided thread to exit.
 * \ingroup group_threads
 */
thread_ret_t thread_join(thread_t t);

/**
 * \brief Causes the current thread to sleep for the supplied number of milliseconds.
 * \ingroup group_threads
 */
void thread_msleep(int milliseconds);

/**
 * \brief Moves the thread to the next CPU. Used to distribute load over all CPUs in the system as more threads launch.
 * \ingroup group_threads
 */
void thread_nextaffinity();

bool_e thread_unittest(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif

