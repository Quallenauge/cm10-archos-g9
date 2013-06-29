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

#ifndef _SOSAL_HASH_H_
#define _SOSAL_HASH_H_

/*!
 * \file
 * \brief A Hash Table implementation which is an array of lists.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>
#include <sosal/list.h>

/*! \brief The key value pair which is the data storage of the hash.
 * \ingroup group_hashes
 */
typedef struct _record_t {
    value_t key;		/*!< The key used to index the hash */
    value_t value;		/*!< The value */
} record_t;

/*! \brief These define the key depth.
 * \ingroup group_hashes
 */
typedef enum _hash_sizes {
    HASH_SIZE_SMALL = 8,	/*!< Uses an 8 bit indexable table. */
    HASH_SIZE_MEDIUM = 12,	/*!< Uses a 12 bit indexable table. */
    HASH_SIZE_LARGE = 16,	/*!< Uses a 16 bit indexable table. */
} hash_size_e;

/*! \brief The hashing function which generates the indexing into the list array.
 * \param [in] numElem The number of elements in the hash table.
 * \param [in] key The key to use to generate the index in the table.
 * \ingroup group_hashes
 */
typedef value_t (*hash_func_f)(size_t numElem, value_t key);

/*! \brief The hash meta data structure.
 * \ingroup group_hashes
 */
typedef struct _hash {
    hash_size_e size;		/*!< \brief The indexing table bit depth. */
    size_t numTotalElem;	/*!< \brief The number of total elements in the hash. */
    size_t numActiveSlots;	/*!< \brief The number of active slots in the hash. */
    size_t numSlots;		/*!< \brief The number of slots in the table. */
    hash_func_f keyFunc;	/*!< \brief The function pointer to the hashing function. */
    list_t **bucket;		/*!< \brief The key indexed table of all lists of elements. */
} hash_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief The function which creates the hash.
 * \param [in] size The enumerated size of the bitdepth of the table. 8, 12, or 16.
 * \param [in] keyFunc the pointer to the custom keygen function is one is desired.
 * The default implementation will be used if NULL is given.
 * \ingroup group_hashes
 */
hash_t *hash_create(hash_size_e size, hash_func_f keyFunc);

/*!
 * \brief This function sets a value into the hash using the key.
 * \param [in] h The hash.
 * \param [in] key The keying value.
 * \param [in] value the pointer to the value to store. If NULL, any existing value is removed from the hash.
 * \ingroup group_hashes
 */
void hash_set(hash_t *h, value_t key, value_t value);

/*!
 * \brief This function retrieves a value from the hash using the key.
 * \param [in] h The hash.
 * \param [in] key The keying value.
 * \param [out] value The pointer to the location to store the value.
 * \ingroup group_hashes
 */
bool_e hash_get(hash_t *h, value_t key, value_t *value);

/*!
 * \brief This function returns each value contained in itself.
 * \param [in] h The hash.
 * \param [out] value the pointer to the location to store the value removed from the hash. Passing NULL will reset the cleaner.
 * \return Returns the number of active elements in the hash. If called successively, this
 * function will "drain" the hash.
 * \ingroup group_hashes
 */
size_t hash_clean(hash_t *h, value_t *value);

/*!
 * \brief This function returns the number of actual entries in the hash (including the collisions).
 * \param [in] h The hash.
 * \return Returns the number of elements in the hash. A -1 indicates an error.
 * \ingroup group_hashes
 */
size_t hash_length(hash_t *h);

/*!
 * \brief This function prints a description of the hash.
 * \param [in] h The hash.
 * \param [in] printElems Give 1 or 0 to choose to print the elements of the hash or not.
 * \ingroup group_hashes
 */
void hash_print(hash_t *h, bool_e printElems);

/*!
 * \brief Destroys the hash.
 * \param [in] h The hash to deinitialize.
 * \ingroup group_hashes
 */
void hash_destroy(hash_t *h);

#ifdef __cplusplus
}
#endif

#endif

