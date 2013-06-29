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

#ifndef _SOSAL_HEAP_H_
#define _SOSAL_HEAP_H_

/*! \file
 * \brief A simple heap allocator.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>
#include <sosal/mutex.h>

/*! \brief The basic unit of an allocation.
 * \ingroup group_heaps
 */
typedef struct _block_t {
    size_t  size;       /*!< \brief The number of bytes in the block */
    void   *data;       /*!< \brief The pointer to the block data */
} block_t;

/*! \brief A heap_t is a set of list_t's of block_t's which manage the heap.
 * \ingroup group_heaps
 */
typedef struct _heap_t {
    void    *raw;           /*!< \brief This is the raw pointer to the memory area */
    list_t   used;          /*!< \brief This is the sorted list of all used blocks */
    list_t   free;          /*!< \brief This is the sorted list of all free blocks */
    list_t   spare;         /*!< \brief This is the sorted list of all spare blocks (not free and not used) */
    block_t *blocks;        /*!< \brief The list of raw blocks (unsorted) */
    node_t  *nodes;         /*!< \brief The list of raw nodes (unlisted) */
    void    *data;          /*!< \brief This is the raw pointer to the heap area */
    size_t   maxBlocks;     /*!< \brief The maximum number of blocks that can created. */
    size_t   align;         /*!< \brief The alignment size in bytes of the allocations */
    size_t   bytesUsed;     /*!< \brief Counts all the used bytes */
    size_t   bytesFree;     /*!< \brief Counts all the free bytes */
    size_t   bytesTotal;    /*!< \brief This is the total for all the bytes in the heap. This includes the overhead sections */
    bool_e   cleanPolicy;   /*!< \brief This determines if the block is cleaned during frees */
    bool_e   coalescePolicy;/*!< \brief This determines if the blocks will be coalesced during free operations */
    mutex_t  mutex;         /*!< \brief The access mutex */
} heap_t;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief This function takes a pointer to a flat area and a byte count, creating a heap.
 * It will 32bit align both the heap and the number of bytes.
 * IE if you give it an odd pointer and an odd number of bytes
 * you will have a less effective size of heap.
 * \note Be sure you give this function 32bit aligned pointers and
 * byte counts.
 * \param [in] heap The pointer to the heap meta-data.
 * \param [in] buffer The begining of the heap
 * \param [in] size The size of buffer in the heap
 * \param [in] p2align Pointer to align
 * \ingroup group_heaps
 */
bool_e heap_init(heap_t *heap, void *buffer, size_t size, uint32_t p2align);

/*!
 * \brief Deconstructor of the heap.
 * \param [in] heap The pointer to the heap meta-data.
 * \post Set your heap pointer to NULL.
 * \pre \ref heap_init
 * \ingroup group_heaps
 */
void heap_deinit(heap_t *heap);

/*!
 * \brief Allocates a block of memory.
 * \param heap The pointer to the heap meta-data.
 * \param num_bytes The number of bytes requested (this will be internally rounded up)
 * \return Returns the allocated pointer or NULL if not enough memory existed or some internal error prevents allocation.
 * \note This will always return an aligned pointer.
 * \pre \ref heap_init
 * \post \ref heap_free
 * \ingroup group_heaps
 */
void *heap_alloc(heap_t *heap, size_t num_bytes);

/*!
 * \brief Frees a buffer within the heap.
 * \param heap The pointer to the heap meta-data.
 * \param pointer The pointer to free
 * \return Returns true if the memory was freed, false otherwise.
 * \pre heap_allocate
 * \pre heap_deinit
 * \ingroup group_heaps
 */
bool_e heap_free(heap_t *heap, void *pointer);

/*!
 * \brief Checks to see if the buffer exists inside this heap area.
 * \param heap Specified heap to check
 * \param addr The address to check
 * \return Returns a boolean, true if on the user heap, false otherwise.
 * \pre heap_allocate
 * \ingroup group_heaps
 */
bool_e heap_valid(heap_t *heap, void *addr);

#ifdef __cplusplus
}
#endif

#endif
