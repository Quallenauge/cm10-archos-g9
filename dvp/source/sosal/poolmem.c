/**
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

/*!
 * \file
 * \brief Static Pool Memory Allocator
 * \author Erik Rainey <erik.rainey@ti.com>
 * \note For compiling and testing in standalone use:\n
 * \code
$ gcc -O3 poolmem.c -o poolmem -DUNITTEST -DSTANDALONE
$ ./poolmem
 * \endcode
 */

#include <sosal/poolmem.h>
#include <sosal/debug.h>

uint8_t poolmem[POOL_MAX_SIZE];

pool_def_t pooldef[] = {
  {20, POOL_IMAGE_SIZE(1,320,240,3)},
  {1,  POOL_BUFFER_SIZE(sizeof(int32_t),1024)},
};

size_t pooldef_size()
{
    uint32_t p;
    size_t size = 0;
    for (p = 0; p < dimof(pooldef); p++)
        size += (pooldef[p].count + sizeof(size_t)) + (pooldef[p].count * pooldef[p].size);
    return size;
}

static size_t *poolmem_slots(uint32_t pool)
{
    uint32_t p = 0;
    size_t offset = 0;
    for (p = 0; p < dimof(pooldef) && p < pool; p++)
        offset += (pooldef[p].count * sizeof(size_t)) + (pooldef[p].count * pooldef[p].size);
    return (size_t *)&poolmem[offset];
}

static void *poolmem_buffer(uint32_t pool, uint32_t slot)
{
    uint32_t p;
    size_t offset = 0;
    for (p = 0; p < dimof(pooldef); p++) {
        // skip over the slots
        offset += (pooldef[p].count * sizeof(size_t));
        // check to see if it matchings this pool
        if (p == pool && slot < pooldef[p].count) {
            offset += (slot * pooldef[p].size);
            break;
        }
        else
            offset += (pooldef[p].count * pooldef[p].size);
    }
    return (void *)&poolmem[offset];
}

#if defined(SOSAL_DEBUG) && defined(UNITTEST)
static size_t poolmem_active()
{
    uint32_t p,s;
    size_t active = 0;
    size_t *slots = NULL;
    for (p = 0; p < dimof(pooldef); p++) {
        slots = poolmem_slots(p);
        for (s = 0; s < pooldef[p].count; s++) {
            if (slots[s] == 1)
                active++;
        }
    }
    return active;
}
#endif

void *poolmem_alloc(size_t size)
{
    uint32_t p,s;
    // make sure the pooldef fits in the poolmem
    assert(pooldef_size() <= sizeof(poolmem));

    // find the pool, then find a slot
    for (p = 0; p < dimof(pooldef); p++)
    {
        if (pooldef[p].size == size)
        {
            size_t *slots = poolmem_slots(p);
            for (s = 0; s < pooldef[p].count; s++)
            {
                if (slots[s] == 0)
                {
                    slots[s] = 1;
                    return poolmem_buffer(p, s);
                }
            }
        }
    }
    return NULL;
}

void poolmem_free(void *ptr)
{
    uint32_t p,s,found=0;
    void *ptrcomp = NULL;
    void *ptrend = &poolmem[POOL_MAX_SIZE];

    if ((void *)poolmem <= ptr && ptr < ptrend)
    {
        for (p = 0; p < dimof(pooldef); p++)
        {
            size_t *slots = poolmem_slots(p);
            for (s = 0; s < pooldef[p].count; s++)
            {
                ptrcomp = poolmem_buffer(p,s);
                SOSAL_PRINT(SOSAL_ZONE_POOLMEM,"Comparing slots[%u]=%zu ptr %p to %p\n", s, slots[s], ptr, ptrcomp);
                if (ptr == ptrcomp && slots[s] == 1)
                {
                    slots[s] = 0;
                    found = 1;
                    break;
                }
            }
            if (found == 1)
              break;
        }
    }
    else
        SOSAL_PRINT(SOSAL_ZONE_ERROR,"%p is out of the poolmem=[%p-%p)!\n", ptr,poolmem,ptrend);
}

void poolmem_print()
{
#if defined(SOSAL_DEBUG)
    uint32_t p,s;
    for (p = 0; p < dimof(pooldef); p++)
    {
        size_t *slots = poolmem_slots(p);
        SOSAL_PRINT(SOSAL_ZONE_POOLMEM,"Pool of %zu, %zu byte slots\n", pooldef[p].count, pooldef[p].size);
        for (s = 0; s < pooldef[p].count; s++)
        {
            void *ptr = poolmem_buffer(p,s);
            SOSAL_PRINT(SOSAL_ZONE_POOLMEM,"\tslot[%u] = %zu, ptr = %p\n",s,slots[s],ptr);
        }
    }
    SOSAL_PRINT(SOSAL_ZONE_POOLMEM,"pooldef_size()=%zu, poolmem_size=%zu\n",pooldef_size(), sizeof(poolmem));
#endif
}

#if defined(UNITTEST)
int poolmem_unittest(int argc, char *argv[])
{
    void *ptr;
    uint32_t p,s;

    SOSAL_PRINT(SOSAL_ZONE_ALWAYS,"pooldef uses %zu byes from poolmem of %zu bytes from %p\n", pooldef_size(), sizeof(poolmem), poolmem);

    // try to allocate an incorrectly size unit
    ptr = poolmem_alloc(2);
    if (ptr) {
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS,"Allocated incorrect memory at %p\n", ptr);
        poolmem_free(ptr);
        return -1;
    }
    else
    {
        SOSAL_PRINT(SOSAL_ZONE_ERROR,"Failed to allocate incorrectly sized memory from poolmem! (This is good)\n");
    }

    for (p = 0; p < dimof(pooldef); p++)
    {
        void **ptrs = NULL;
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS,"Pool[%u] has %zu slots\n",p,pooldef[p].count);
        ptrs = (void **)calloc(pooldef[p].count, sizeof(void *));
        for (s = 0; s <= pooldef[p].count; s++)
        {
            // last alloc should fail
            ptrs[s] = poolmem_alloc(pooldef[p].size);
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS,"\tslot[%u]=%p\n", s, ptrs[s]);
        }
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS,"poolmem_active()=>%zu\n", poolmem_active());
        for (s = pooldef[p].count-1; s != -1; s--)
        {
            poolmem_free(ptrs[s]);
        }
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS,"poolmem_active()=>%zu\n", poolmem_active());
    }
    poolmem_print();
}

#if defined(STANDALONE)
int main(int argc, char *argv[])
{
    return poolmem_unittest(argc, argv);
}
#endif
#endif
