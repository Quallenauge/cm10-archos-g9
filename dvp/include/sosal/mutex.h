/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

#ifndef _SOSAL_MUTEX_H_
#define _SOSAL_MUTEX_H_

/*! \file  
 * \brief The SOSAL Mutex API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>

#ifdef POSIX

/*! \brief On POSIX system we wrap pthread_mutex_t. 
 * \ingroup group_mutexes
 */
typedef pthread_mutex_t mutex_t;
#define MUTEX_INITIAL   PTHREAD_MUTEX_INITIALIZER

#elif defined(SYSBIOS)

#include <ti/sysbios/gates/GateMutexPri.h>
/*! \brief On SYSBIOS we wrap the GateMutexPri as it implements priority 
 * inheritance to protect against priority inversion.
 * \ingroup group_mutexes
 */
typedef struct TIMM_OSAL_MUTEX {
    GateMutexPri_Handle gate;
    IArg                key;
} TIMM_OSAL_MUTEX;

/*! \brief Wrap another mutex thinly on SYSBIOS.
 * \ingroup group_mutexes
 */
typedef TIMM_OSAL_MUTEX mutex_t;
#define MUTEX_INITIAL 	{0,0}

#else
typedef CRITICAL_SECTION mutex_t;
#define MUTEX_INITIAL   {0}
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Initializes the mutex value.
 * \param [in] m The pointer to the mutex.
 * \post \ref mutex_deinit
 * \ingroup group_mutexes
 */
void mutex_init(mutex_t *m);

/*! \brief Deinitializes the mutex.
 * \param [in] m The pointer to the mutex.
 * \pre \ref mutex_init
 * \ingroup group_mutexes
 */
void mutex_deinit(mutex_t *m);

/*! \brief This locks access to the mutex and causes this thread to block if 
 * already acquired. 
 * \param [in] m The pointer to the mutex.
 * \pre \ref mutex_init
 * \post \ref mutex_unlock
 * \ingroup group_mutexes
 */
void mutex_lock(mutex_t *m);

/*! \brief This unlocks access to the mutex and causes other threads which 
 * were block to unblock. 
 * \param [in] m The pointer to the mutex.
 * \pre \ref mutex_lock
 * \post \ref mutex_deinit
 * \ingroup group_mutexes
 */
void mutex_unlock(mutex_t *m);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace SOSAL {
    /*! \brief The SOSAL C++ which wraps the mutex to permit "auto" locking by
     * using C++'s autoinstancing and destruction of objects in a scope.
     * \ingroup group_mutexes.
     */
    class AutoLock {
    private:
        mutex_t *m_lock;
    public:
        /*! \brief A grabs the lock */
        AutoLock(mutex_t *m) {
            m_lock = m;
            mutex_lock(m_lock);
        }
        /*! \brief Releases the lock. 
         * \note This is called automatically by the compiler.
         */
        ~AutoLock() {
            mutex_unlock(m_lock);
        }
    };
}
#endif

#endif

