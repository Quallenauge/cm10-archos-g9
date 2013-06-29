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

#include <sosal/list.h>
#include <sosal/heap.h>
#include <sosal/debug.h>

#if SOSAL_ZONE_EXTRA != 0
#define SOSAL_EXTRA_DEBUG
#endif

//******************************************************************************
// INTERNAL FUNCTIONS
//******************************************************************************

static bool_e ptr_is_aligned(void *ptr, size_t align)
{
    size_t addr = (size_t)ptr;
    if ((addr % align) == 0)
        return true_e;
    else
        return false_e;
}

static void node_clear(node_t *node)
{
    if (node)
    {
        node->prev = NULL;
        node->next = NULL;
        node->data = 0;
    }
}

static void block_clear(block_t *block)
{
    if (block)
    {
        block->data = 0;
        block->size = 0;
    }
}

#ifdef SOSAL_EXTRA_DEBUG
static void block_printer(node_t *node)
{
    block_t *block = (block_t *)node->data;
    if (block) {
        SOSAL_PRINT(SOSAL_ZONE_EXTRA, "Node %p Block %p Data %p for "FMT_SIZE_T" bytes\n", node, block, block->data, block->size);
    } else {
        SOSAL_PRINT(SOSAL_ZONE_EXTRA, "Node %p Block %p\n", node, block);
    }
}
#endif

/** This will generate the correct block size for the desired block taking the alignment and possible padding into account. */
static size_t block_size(size_t align, size_t desired)
{
    size_t remainder = desired % align;
    if (remainder != 0)
    {
        desired += (align - remainder);
    }
    return desired;
}


static int block_compare_ptr(node_t *a, node_t *b)
{
    if (b != NULL)
    {
        block_t *blockA = (block_t *)a->data;
        block_t *blockB = (block_t *)b->data;

        if (blockA->data < blockB->data)
        {
            return -1;
        }
        else if (blockA->data == blockB->data)
        {
            return 0;
        }
        else if (blockA->data > blockB->data)
        {
            return 1;
        }
    }
    return (0);
}

static bool_e block_is_adjacent(block_t *a, block_t *b)
{
    if (a == NULL || b == NULL)
        return false_e;

    if ((uint8_t *)a->data + a->size == (uint8_t *)b->data)
        return true_e;
    else
        return false_e;
}

static score_t block_rate_bestfit(node_t *current, node_t *criteria)
{
    if (current && criteria)
    {
        block_t *this_block = (block_t *)current->data;
        block_t *comp_block = (block_t *)criteria->data;

        if (this_block->size == comp_block->size)
            return MAX_SCORE;
        else if (this_block->size > comp_block->size)
            return MAX_SCORE - (this_block->size - comp_block->size);
        else
            return MIN_SCORE;
    }
    else
        return MIN_SCORE;
}

static bool_e block_bestfit_and_remove(list_t *list, node_t **node, block_t **block, size_t size)
{
    block_t searchBlock = {size, NULL};
    node_t  searchNode  = {NULL, NULL, (value_t)&searchBlock};
    node_t *foundNode;

#ifdef SOSAL_EXTRA_DEBUG
    SOSAL_PRINT(SOSAL_ZONE_HEAP, "Scoring Free List\n");
    list_print(list, block_printer);
#endif
    foundNode = list_score(list, &searchNode, block_rate_bestfit);
    if (foundNode != NULL) {
        *node = list_extract(list, foundNode);
        *block = (block_t *)(*node)->data;
        return true_e;
    }
    else
        return false_e;
}

static bool_e block_find_and_remove(list_t *list, node_t **node, block_t **block, void *ptr)
{
    block_t searchBlock;
    node_t  searchNode;
    node_t *foundNode;

    // find the block which controls this pointer
    searchBlock.data = ptr;
    searchBlock.size = 0;
    searchNode.data = (value_t)&searchBlock;
    searchNode.prev = NULL;
    searchNode.next = NULL;

    // confirm that the block is in the free list
    foundNode = list_remove_match(list, &searchNode, block_compare_ptr);
    if (foundNode)
    {
        if (node != NULL)
            *node = foundNode;
        if (block != NULL)
            *block = (block_t *)foundNode->data;
        return true_e;
    }
    else
        return false_e;
}

static bool_e heap_calc_overhead(heap_t *heap)
{
    // this is the cost of keeping track of the allocations
    size_t bookkeeping = sizeof(node_t) + sizeof(block_t);

    // this is the ratio of the overhead to the total
    // this equation is derived from the design of the heap.
    float ratio = ((float)heap->align/bookkeeping) + 1.0f;

    // recast the overhead from a float
    size_t overhead = (size_t)((float)heap->bytesTotal/ratio);

    // the number of bookkeeping elements
    size_t num = (overhead/bookkeeping);

    // the size of the data region of the heap
    size_t data = 0;

    // the number of bytes between the overhead and the data, used to align the data pointer.
    size_t pad = 0;

    if (num == 0)
        return false_e;

    do {
        overhead = num * bookkeeping;
        data = num * heap->align;
        pad = heap->bytesTotal - data - overhead;

        SOSAL_PRINT(SOSAL_ZONE_HEAP, "Heap: "FMT_SIZE_T" total = "FMT_SIZE_T" overhead + "FMT_SIZE_T" data ("FMT_SIZE_T" pad) ("FMT_SIZE_T" num) ("FMT_SIZE_T" sum)\n", heap->bytesTotal, overhead, data, pad, num, (pad+data+overhead));

        if (heap->bytesTotal >= (pad+data+overhead))
        {
            // setup the block pointer
            heap->blocks = (block_t*)heap->raw;

            // setup the node pointer right afterwards
            heap->nodes  = (node_t *)((uint8_t *)heap->raw + num*sizeof(block_t));

            // setup the data pointer initially
            heap->data = (uint8_t *)heap->raw + overhead;

            // setup the tallies
            heap->bytesUsed = 0;
            heap->bytesFree = data;
            heap->maxBlocks = num;

            // align the data pointer if it is not
            if (ptr_is_aligned(heap->data, heap->align) == false_e)
            {
                // offset the data pointer if it is not aligned (the pad should have enough room)
                size_t offset = (heap->align - ((size_t)heap->data % heap->align));
                if (offset > pad)
                {
                    SOSAL_PRINT(SOSAL_ZONE_WARNING, "WARNING! Pad is too small to account for offset for alignment!\n");
                    num--;
                    pad = heap->bytesTotal;
                }
                else
                {
                    SOSAL_PRINT(SOSAL_ZONE_HEAP, "Offsetting data by "FMT_SIZE_T" bytes\n",offset);
                    heap->data = (uint8_t *)heap->data + offset;
                    return true_e;
                }
            }
        }
        else
            num--;
    } while (heap->bytesTotal < (pad+data+overhead));
    return true_e;
}

/*******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************/

/**
 * This function takes a pointer to a heap and a byte count.
 * It will 32bit align both the heap and the number of bytes.
 * IE if you give it an odd pointer and an odd number of bytes
 * you will have a less effective size of heap.
 * Be sure you give this function 32bit aligned pointers and
 * byte counts.
 * @param heap
 * @param buffer The begining of the heap
 * @param size The size of buffer in the heap
 * @param p2align Pointer to align
 */
bool_e heap_init(heap_t *heap, void *buffer, size_t size, uint32_t p2align)
{
    uint32_t spareIdx = 0;
    node_t *first_node;
    block_t *first_block;

    if (p2align > 12)
        p2align = 12;
    if (p2align < 5)
        p2align = 5;
    // alignments can only be between 32 bytes and 4096 bytes.

    if (heap != NULL)
        memset(heap, 0, sizeof(heap_t));
    else
        return false_e;

    // initialize our values
    heap->bytesTotal = size;
    heap->cleanPolicy = true_e;
    heap->coalescePolicy = true_e;
    heap->align = (1<<p2align);
    heap->raw = buffer;
    mutex_init(&heap->mutex);

    if (buffer == NULL)
        return false_e;

    if (ptr_is_aligned(buffer, 4) == false_e) // we just require 32 bit alignment
        return false_e;

    // initialize the entire data space of the heap
    memset(buffer, 0, size);

    // calculates and sets up the overhead and data areas
    if (heap_calc_overhead(heap) == false_e)
        return false_e;

    // initializes the nodes and blocks to the spare list
    for (spareIdx = 0; spareIdx < heap->maxBlocks; spareIdx++)
    {
        node_t *this_node  = &heap->nodes[spareIdx];
        block_t *this_block = &heap->blocks[spareIdx];

        if (spareIdx > 0)
            this_node->prev = &heap->nodes[spareIdx - 1];
        else {
            heap->spare.head = this_node;
            this_node->prev = NULL;
        }
        if (spareIdx < (heap->maxBlocks - 1))
            this_node->next = &heap->nodes[spareIdx + 1];
        else {
            heap->spare.tail = this_node;
            this_node->next = NULL;
        }
        this_node->data = (value_t)this_block;
        this_block->data = NULL;
        this_block->size = 0;
    }

    // get the first node
    first_node = list_pop(&heap->spare);
    first_block = (block_t *)first_node->data;

    // set the first block to the size of the entire free heap
    first_block->size = heap->bytesFree;
    first_block->data = heap->data;

    // add this to the free list
    list_push(&heap->free, first_node);

    // this block will be chopped up later for smaller allocations.

    SOSAL_PRINT(SOSAL_ZONE_HEAP, "heap_init(): heap_t *%p initialized with "FMT_SIZE_T" bytes at %p\n",heap,heap->bytesTotal,heap->raw);
    return true_e;
}


/**
 * Deconstructor
 */
void heap_deinit(heap_t *heap)
{
    mutex_deinit(&heap->mutex);
    memset(heap->raw, 0, heap->bytesTotal);
    memset(heap, 0, sizeof(heap_t));
}

/**
 * Allocates a block of memory.
 * @param heap
 * @param num_bytes The number of bytes requested (this will be internally rounded up)
 * @return Returns the allocated pointer or NULL if not enough memory existed or some internal error prevents allocation.
 * NOTE - This will always return an aligned pointer.
 */
void *heap_alloc(heap_t *heap, size_t num_bytes)
{
    void *ptr = NULL;
    block_t *block = NULL;
    node_t  *node = NULL;

    if (heap->raw == NULL)
        return NULL;

    if (num_bytes == 0)
        return NULL;

    // round up to the actual amount of bytes we'll allocate
    num_bytes = block_size(heap->align, num_bytes);

    if (heap->bytesFree < num_bytes)
        return NULL;

    mutex_lock(&heap->mutex);

#ifdef SOSAL_EXTRA_DEBUG
    // show current allocations
    SOSAL_PRINT(SOSAL_ZONE_EXTRA, "Used List:\n");
    list_print(&heap->used, block_printer);
#endif

    // find the best fitting free block to reuse
    if (block_bestfit_and_remove(&heap->free, &node, &block, num_bytes) == false_e)
    {
        // didn't find one
        mutex_unlock(&heap->mutex);
        return NULL;
    }
    else if (node && block)
    {
        // did the best fit have too many bytes?
        if (block->size > num_bytes)
        {
            //  get a node/block to hold the remainder
            node_t *spareNode = list_pop(&heap->spare);
            block_t *spareBlock = NULL;

            SOSAL_PRINT(SOSAL_ZONE_HEAP, "Fracturing Node %p Block %p\n", node, block);
            if (spareNode == NULL) // ran out of meta-data
            {
                // put the node back on the free list
                list_insert(&heap->free, node, block_compare_ptr, true_e);
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "FATAL ERROR! Not enough spare nodes to maintain data structures!\n");
                mutex_unlock(&heap->mutex);
                return NULL;
            }

            // get the block pointer
            spareBlock = (block_t *)spareNode->data;

            // fracture the larger block into the spare block
            spareBlock->data = ((uint8_t*)block->data + num_bytes);
            spareBlock->size = (block->size - num_bytes);

            SOSAL_PRINT(SOSAL_ZONE_HEAP, "Spare Block %p has pointer %p and size "FMT_SIZE_T"\n", spareBlock, spareBlock->data, spareBlock->size);

            // now adjust the block
            block->size = num_bytes;

            // put the remainder on the free list
            list_insert(&heap->free, spareNode, block_compare_ptr, true_e);
        }

        if (ptr_is_aligned(block->data, heap->align) == false_e) {
            SOSAL_PRINT(SOSAL_ZONE_WARNING, "WARNING! The block pointer is not aligned!\n");
        }

        // put the allocated node/block on the used list
        list_insert(&heap->used, node, block_compare_ptr, true_e);

        // update the global values
        heap->bytesFree -= num_bytes;
        heap->bytesUsed += num_bytes;

        ptr = block->data;

        mutex_unlock(&heap->mutex);

        return ptr;
    }
    else
    {
        mutex_unlock(&heap->mutex);
        return NULL;
    }
}

/**
 * Frees a buffer within the heap.
 * @param heap
 * @param pointer The pointer to free
 */
bool_e heap_free(heap_t *heap, void *pointer)
{
    block_t *block;
    node_t *node;

    if (pointer == NULL)
        return false_e;

    if (heap->raw == NULL)
        return false_e;

    if (ptr_is_aligned(pointer, heap->align) == false_e)
        return false_e;

    mutex_lock(&heap->mutex);

    // show current allocations
    //SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Free List:\n");
    //list_print(&heap->free, block_printer);

    //SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Used List:\n");
    //list_print(&heap->used, block_printer);

    if (block_find_and_remove(&heap->used, &node, &block, pointer) == false_e)
    {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR could not find the node/block/pointer in the used list!\n");
        mutex_unlock(&heap->mutex);
        return false_e;
    }

    // this is not the pointer we wanted.
    if (pointer != block->data)
    {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Block was misidentified!\n");
        mutex_unlock(&heap->mutex);
        return false_e;
    }

    // insert to the free list (in sorted order)
    list_insert(&heap->free, node, block_compare_ptr, true_e);

    SOSAL_PRINT(SOSAL_ZONE_HEAP, "heap_free(): Freed Node %p Block %p Pointer %p for "FMT_SIZE_T" bytes\n", node, block, block->data, block->size);

    // upkeep the global variables
    heap->bytesUsed -= block->size;
    heap->bytesFree += block->size;

    // clean the block
    if (heap->cleanPolicy)
        memset(block->data, '0', block->size);

    /// @todo can the free list be coalesced at this point? Yes. There should be at most 2 operations to do.
    if (heap->coalescePolicy)
    {
        node_t  *prev_node = node->prev;
        node_t  *next_node = node->next;
        block_t *prev_block = (prev_node?(block_t *)prev_node->data:NULL);
        block_t *next_block = (next_node?(block_t *)next_node->data:NULL);
        if (block_is_adjacent(prev_block, block) == true_e)
        {
            block->data = prev_block->data;
            block->size += prev_block->size;
            block_clear(prev_block);

            prev_node->data = (value_t)block;
            prev_node->next = node->next;
            if (node->next)
                node->next->prev = prev_node;
            node_clear(node);
            SOSAL_PRINT(SOSAL_ZONE_HEAP, "Coalesced with previous node\n");
        }
        else if (block_is_adjacent(block, next_block) == true_e)
        {
            block->size += next_block->size;
            block_clear(next_block);

            node->next = next_node->next;
            if (next_node->next)
                next_node->next->prev = node;
            node_clear(next_node);
            SOSAL_PRINT(SOSAL_ZONE_HEAP, "Coalesced with next node\n");
        }
    }
    mutex_unlock(&heap->mutex);
    return true_e;
}

bool_e heap_valid(heap_t *heap, void *addr)
{
    bool_e ret = false_e;
    mutex_lock(&heap->mutex);
    if (heap->raw)
    {
        uint8_t *a = heap->raw;
        size_t s = heap->bytesTotal;
        uint8_t *b = (uint8_t *)addr;
        uint8_t *c = (uint8_t *)(a+s);
        if (a <= b && b < c)
            ret = true_e;
    }
    mutex_unlock(&heap->mutex);
    return ret;
}

bool_e heap_unittest_overfill(void *buffer, size_t size)
{
    bool_e ret = true_e;
    heap_t h;
    uint32_t align = 7;
    size_t unitsize = (1<<align);
    if (heap_init(&h, buffer, size, align) == true_e)
    {
        uint32_t i;
        uint32_t num = h.maxBlocks + 1;
        void **bufs = calloc(sizeof(void *),num);
        for (i = 0; i < num; i++)
        {
            bufs[i] = heap_alloc(&h, unitsize);
            if (bufs[i] == NULL)
            {
                if (i == (num - 1)) {
                    ret = true_e;
                }
                else {
                    ret = false_e;
                    break;
                }
            }
        }
        for (i = 0; i < num; i++)
        {
            heap_free(&h, bufs[i]);
        }
        free(bufs);
        heap_deinit(&h);
    }
    return ret;
}

bool_e heap_unittest_iterations(void *buffer, size_t size)
{
    bool_e ret = true_e;
    heap_t h;
    uint32_t align = 7;
    uint32_t iterations = 100;
    uint32_t unitsizelimit = (1 << align) * 3;
    uint32_t i,j;
    if (heap_init(&h, buffer, size, align) == true_e)
    {
        void *bufs[10];
        //SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Heap Iterations test:\n");
        for (i = 0; i < iterations; i++)
        {
            memset(bufs, 0, sizeof(bufs));
            for (j = 0; j < dimof(bufs); j++)
            {
                uint32_t unitsize = (rand()%unitsizelimit)+1;
                bufs[j] = heap_alloc(&h, unitsize);
                if (bufs[j] == NULL)
                {
                    SOSAL_PRINT(SOSAL_ZONE_HEAP, "ERROR! Failed to allocate a buffer of %u bytes\n", unitsize);
                    ret = false_e;
                }
                if (heap_valid(&h, bufs[j]) == false_e)
                {
                    SOSAL_PRINT(SOSAL_ZONE_HEAP, "ERROR! Pointer %p is not located in the heap!\n", bufs[j]);
                    ret = false_e;
                }
            }
            for (j = 0; j < dimof(bufs); j++)
            {
                if (heap_free(&h, bufs[j]) == false_e)
                    ret = false_e;
            }
            if (h.bytesUsed != 0)
            {
                SOSAL_PRINT(SOSAL_ZONE_HEAP, "ERROR! Outstanding allocations!\n");
                ret = false_e;
            }
            if (ret == false_e)
                break;
            //SOSAL_PRINT(SOSAL_ZONE_ALWAYS, ".");fflush(stdout);
        }
        //SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "\n");
        heap_deinit(&h);
    }
    if (ret == false_e)
        SOSAL_PRINT(SOSAL_ZONE_HEAP, "Iterations failed on %u\n", i);
    return ret;
}

bool_e heap_unittest_fragmentation(void *buffer, size_t size)
{
    bool_e ret = true_e;
    uint32_t p2align = 7;
    uint32_t failures = 0;
    void *temp = NULL;
    heap_t h;
    if (heap_init(&h, buffer, size, p2align) == true_e)
    {
        void **bufs = calloc(sizeof(void*),h.maxBlocks);
        if (bufs)
        {
            uint32_t i = 0;
            for (i = 0; i < h.maxBlocks; i++) {
                bufs[i] = heap_alloc(&h, (1<<p2align));
                if (bufs[i] == NULL)
                    failures++;
            }
            for (i = 0; i < h.maxBlocks; i+=2) {
                if (heap_free(&h, bufs[i]) == false_e)
                    failures++;
            }
            // test for a failure to allocate a double sized block
            temp = heap_alloc(&h, (1<<(p2align+1)));
            if (temp != NULL) {
                failures++;
                ret = false_e;
                heap_free(&h, temp);
            }

            for (i = 1; i < h.maxBlocks; i+=2) {
                if (heap_free(&h, bufs[i]) == false_e)
                    failures++;
            }

        }
        free(bufs);
        heap_deinit(&h);
    }
    else
        ret = false_e;
    if (failures > 0) {
        ret = false_e;
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "heap fragmentation test; %u faiiures\n", failures);
    }
    return ret;
}

bool_e heap_unittest_simple(void *buffer, size_t size)
{
#define numBufs 10
    bool_e ret = true_e;
    heap_t h;
    uint32_t i = 0;
    void *bufs[numBufs];
    size_t bufSizes[numBufs];

    if (heap_init(&h, buffer, size, 7) == true_e) // 128 byte align
    {
        for (i = 0; i < numBufs; i++)
        {
            // all the allocations should fit
            do {
                bufSizes[i] = rand() % (size/numBufs);
            } while (bufSizes[i] == 0);
            bufs[i] = heap_alloc(&h, bufSizes[i]);
            if (bufs[i] == NULL)
                ret = false_e;
            //SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Buf[%u]=%p for %u bytes\n", i, bufs[i], bufSizes[i]);
        }

        for (i = 0; i < numBufs; i++)
        {
            if (bufs[i] != NULL && heap_valid(&h, bufs[i]) == false_e)
            {
                ret = false_e;
                SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "FAILED TO VALIDATE KNOWN GOOD BUFFER %p!\n", bufs[i]);
            }
        }

        for (i = 0; i < numBufs; i++)
        {
            if (heap_free(&h, bufs[i]) == false_e)
            {
                SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "FAILED to free %p\n", bufs[i]);
                ret = false_e;
            }
        }

        heap_deinit(&h);
    }
    else
        ret = false_e;
    return ret;
}

bool_e heap_unittest(int argc __attribute__((unused)),
                     char *argv[] __attribute__((unused)))
{
    bool_e ret = true_e;
    size_t size = 1<<20;
    void *flatbuffer = calloc(1,size);
    uint32_t i;
    bool_e verbose = false_e;
    if (flatbuffer)
    {
        if (heap_unittest_simple(flatbuffer, size) == false_e)
            ret = false_e;
        if (heap_unittest_iterations(flatbuffer, size) == false_e)
            ret = false_e;
        if (heap_unittest_overfill(flatbuffer, size) == false_e)
            ret = false_e;
        if (heap_unittest_fragmentation(flatbuffer, size) == false_e)
            ret = false_e;
        free(flatbuffer);
        flatbuffer = NULL;
    }
    if (verbose)
    {
        size_t sizes[] = {23, 14, 80, 129, 4097, 2304, 9027, 82734};
        for (i = 0; i < dimof(sizes); i++)
        {
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "size "FMT_SIZE_T" is rounded to "FMT_SIZE_T"\n", sizes[i], block_size(128, sizes[i]));
        }
    }
    return ret;
}


//******************************************************************************
// END
//******************************************************************************
