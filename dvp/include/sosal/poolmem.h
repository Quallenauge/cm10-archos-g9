/*
 *  Copyright (C) 2012-2013 Texas Instruments, Inc.
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

#ifndef _SOSAL_POOLMEM_H_
#define _SOSAL_POOLMEM_H_

/*!
 * \file
 * \brief Static Pool Memory Allocator
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>

#define POOL_IMAGE_SIZE(bpp,ppl,lpi,ppi) (bpp*ppl*lpi*ppi)
#define POOL_BUFFER_SIZE(bpu,nu)         (bpu*nu)
#define POOL_STRUCT(structure)           (sizeof(structure))

typedef struct _pool_def_t {
    size_t count;
    size_t size;
} pool_def_t;

#ifndef POOL_MAX_SIZE
#define POOL_MAX_SIZE (1<<23) // 8MB
#else
//#pragma message("Using command line provided pool size!\n")
#endif

/*! \brief Returns the size of the pool as required by the pool definition.
 * \ingroup group_sosal_pool
 */
size_t pooldef_size();

/*! \brief Returns a opinter to a preallocated slot in the pool memory if there
 * is space available.
 * \param [in] size The byte size required.
 * \return Returns a poitner to a preallocated slot.
 * \note This will assert if the defined size of the pool is smaller than the
 * actually memory assigned to the pool.
 * \ingroup group_sosal_pool
 */
void *poolmem_alloc(size_t size);

/*! \brief This frees the slot in the pool indicated by the pointer.
 * \ingroup group_sosal_pool
 */
void poolmem_free(void *ptr);

/*! \brief This prints the values of the pool memory state.
 * \ingroup group_sosal_pool
 */
void poolmem_print();

#endif

