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

#ifndef _SOSAL_SEMAPHORE_H_
#define _SOSAL_SEMAPHORE_H_

/*! \file
 * \brief The SOSAL Semaphore API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */
 
#include <sosal/types.h>

#if defined(POSIX)

/*! \brief On POSIX systems this wraps sem_t. 
 * \ingroup group_semaphores
 */
typedef sem_t semaphore_t;

#elif defined(SYSBIOS)

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
/*! \brief On BIOS systems this wraps Semaphore_Handle. 
 * \ingroup group_semaphores
 */
typedef Semaphore_Handle semaphore_t;

#elif defined(WIN32)

/*! \brief On Windows systems this wraps a HANDLE. 
 * \ingroup group_semaphores
 */
typedef HANDLE semaphore_t;

#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This function creates a semaphore. 
 * \param [in] s The pointer to a structure to store the semaphore.
 * \param [in] count The initial count to give the semaphore
 * \param [in] shared Indicates if the semaphore is to be shared across processes.
 * \return Returns a bool_e.
 * \retval true_e Indicates that the semaphore was created.
 * \retval false_e indicates that the semaphore was not created.
 * \post \ref semaphore_delete 
 * \ingroup group_semaphores
 */
bool_e semaphore_create(semaphore_t *s, uint32_t count, bool_e shared);

/*! \brief This function deletes a created semaphore.
 * \param [in] s The storage location of the semaphore.
 * \return Returns a bool_e.
 * \retval true_e The semaphore was destroyed.
 * \retval false_e The semaphore was not destroyed.
 * \pre \ref semaphore_create
 * \post Free the pointer if allocated, if an automatic, do nothing.
 * \ingroup group_semaphores
 */
bool_e semaphore_delete(semaphore_t *s);

/*! \brief This function decreases the current count. This is used by 
 * threads or processes after a protected operation is done.
 * \param [in] s The pointer to the semaphore structure.
 * \return Returns a bool_e.
 * \retval true_e if posted.
 * \retavl false_e if an error occurred.
 * \pre \ref semaphore_wait
 * \ingroup group_semaphores
 */
bool_e semaphore_post(semaphore_t *s);

/*! \brief This function blocks until the current count of the semaphore is the 
 * desired count. If the current count is less than or equal to desired count 
 * then enters the semaphore increasing it's count.
 * \param [in] s The pointer to the semaphore structure.
 * \return Returns a bool_e.
 * \retval true_e The semaphore was acquired.
 * \retval false_e The semaphore was not acquired. 
 * \pre \ref semaphore_create
 * \ingroup group_semaphores
 */
bool_e semaphore_wait(semaphore_t *s);

/*! \brief This function tries to grab the semaphore. If 
 * the current count is not the count, the function returns and does not block. 
 * If the current count is less than or equal to the desired count then the 
 * semaphore  is acquired. 
 * \param [in] s The pointer to the semaphore structure.
 * \return Returns a bool_e.
 * \retval true_e The semaphore was acquired.
 * \retval false_e The semaphore was not acquired. 
 * \pre \ref semaphore_create
 * \ingroup group_semaphores
 */
int semaphore_trywait(semaphore_t *s);

#ifdef __cplusplus
}
#endif

#endif

