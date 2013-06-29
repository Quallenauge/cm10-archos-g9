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

#ifndef _SOSAL_BITFIELD_H_
#define _SOSAL_BITFIELD_H_

/*!
 * \file bitfield.h
 * \brief The Bitfield Type API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>

/*! \brief An extensible bitfield (more than architecture width).
 * \ingroup group_bitfields
 */
typedef struct _bitfield_t {
    uint32_t *fields;           /*!< The bitfields array */
    uint32_t max;               /*!< The maximum number of bits that can be set */
    uint32_t count;             /*!< The current number of bits set */
} bitfield_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Initializes a preallocated \ref bitfield_t with a max number of bits
 * \param b The pointer to the bitfield_t structure to initialize.
 * \param max The maximum number of bit needed.
 * \ingroup group_bitfields
 */
void bitfield_init(bitfield_t *b, uint32_t max);

/*! \brief Deinitializes a \ref bitfield_t
 * \param b The pointer to the bitfield_t structure to initialize.
 * \ingroup group_bitfields
 */
void bitfield_deinit(bitfield_t *b);

/*! \brief Lowers bit at index specified
 * \param b The pointer to the bitfield_t structure to initialize.
 * \param bit The bit to clear.
 * \ingroup group_bitfields
 */
bool_e bitfield_rls(bitfield_t *b, uint32_t bit);

/*! \brief Raises bit at index specific unless already raised.
 * \param b The pointer to the bitfield_t structure to initialize.
 * \param bit The pointer to the place to store an acquired bit.
 * \ingroup group_bitfields
 */
bool_e bitfield_get(bitfield_t *b, uint32_t *bit);

/*! \brief Returns the number of bits set
 * \param b The pointer to the bitfield_t structure to initialize.
 * \ingroup group_bitfields
 */
uint32_t bitfield_count(bitfield_t *b);

#ifdef __cplusplus
}
#endif

#endif

