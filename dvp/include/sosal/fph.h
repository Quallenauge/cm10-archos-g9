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

#ifndef _SOSAL_FPH_H_
#define _SOSAL_FPH_H_

/*!
 * \file fph.h
 * \brief The Fast Pointer Hash API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

/*! \brief The recommended size of a FPH block.
 * \ingroup group_fph
 */
#define FPH_BLOCK_SIZE  (256)

/*! \brief The recommended number of blocks in a FPH.
 * \ingroup group_fph
 */
#define FPH_NUM_BLOCKS  (256)

/*! \brief Enables some math optimizations to make the hash faster but with a trade-off
 * of more rigid size requirements.
 * \ingroup group_fph
 */
#undef FPH_OPTIMIZED

/*! \brief Enables or disabled metrics gathering.
 * \ingroup group_fph
 */
#define FPH_METRICS

#include <sosal/types.h>

/*!
 * \brief The key/value pair structure for a Fast Pointer Hash.
 * \ingroup group_fph
 */
typedef struct _fph_kv_t
{
    /*! The key used to access this value pair */
    ptr_t key;
    /*! The value which is stored at the hashed location */
    ptr_t value;
} fph_kv_t;

/*! \brief The function type for the hashing functions for the Fast Pointer Hash.
 * \ingroup group_fph
 */
typedef value_t (*fph_key_f)(void *fph, ptr_t value);

/*!
 * \brief The Fast Pointer Hash data structure.
 * \ingroup group_fph
 */
typedef struct _fph_t
{
    fph_kv_t *hash;             /*!< Pointer to the raw array of value pairs */
    size_t blockSize;           /*!< The initialized number of blocks in the hash */
    size_t numBlocks;           /*!< The initialized size of each block */
    size_t numElem;             /*!< The total number of elements in the hash */
    fph_key_f keyFunc;          /*!< The pointer to the hashing function */
#ifdef FPH_METRICS
    uint32_t *collisions;       /*!< The total number of collisions in each block of the hash */
    uint32_t *length;           /*!< The current number of items in each block of the hash */
    uint32_t numReplacements;   /*!< The number of times any value pair has been replaced */
    uint32_t numPtrs;           /*!< The total number of pointers in the hash */
    uint32_t numMisses;         /*!< The number of misses, pointers which could not be remembered */
#endif
} fph_t;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Initializes the Fast Pointer Hash.
 * \param [in] numBlocks The number of blocks in the hash.
 * \param [in] blockSize The size of each block (count * sizeof(ptr_t)).
 * \param [in] keyFunc A user provided hashing function to generate indexes into the hash.
 * \note If you write your own hashing function, you'll need to use the numBlocks and blockSize in the hash structure.
 * \ingroup group_fph
 */
fph_t *fph_init(size_t numBlocks, size_t blockSize, fph_key_f keyFunc);

/*!
 * \brief Deinitializes the Fast Pointer Hash data structures.
 * \param [in] fph The pointer to the fast pointer hash data structure.
 * \pre \ref fph_init
 * \ingroup group_fph
 */
void fph_deinit(fph_t *fph);

/*!
 * \brief Retrieves the value at the desired key location.
 * \param [in] fph The pointer to the Fast Pointer Hash.
 * \param [in] k The key
 * \param [out] v The pointer to the location to store the value.
 * \ingroup group_fph
 */
int fph_get(fph_t *fph, ptr_t k, ptr_t *v);

/*!
 * \brief Clears the value at the location indicated by the key.
 * \param [in] fph The pointer to the Fast Pointer Hash.
 * \param [in] k The key.
 * \ingroup group_fph
 */
int fph_clr(fph_t *fph, ptr_t k);

/*!
 * \brief Sets the key/value pair in the hash.
 * \param [in] fph The pointer to the Fast Pointer Hash.
 * \param [in] k The key
 * \param [in] v The value
 * \ingroup group_fph
 */
int fph_set(fph_t *fph, ptr_t k, ptr_t v);

#ifdef __cplusplus
}
#endif

#endif // _FPH_H_
