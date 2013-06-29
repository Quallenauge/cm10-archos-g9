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

#include <sosal/types.h>
#include <sosal/debug.h>
#include <sosal/fph.h>

/*!
 * The pre-supplied hashing function for the Fast Pointer Hash.
 * \param fph
 * \param value
 * \see fph_key_f
 */
value_t fph_hash(void *fph, ptr_t value)
{
    value_t hash = (value_t)value;
    //hash &= 0xFFFFFF00;
    hash >>= 8;
#ifndef FPH_OPTIMIZED
    hash %= ((fph_t *)fph)->numBlocks;
#else
    hash &= (((fph_t *)fph)->numBlocks - 1);   // assumes 2^n numBlocks
#endif
    hash *= ((fph_t *)fph)->blockSize;
    //SOSAL_PRINT(SOSAL_ZONE_HASH, "%p => %u\n", value, hash);
    return hash;
}

#ifdef FPH_OPTIMIZED
/*!
 * Determines if a size_t is a power of two.
 * \param a The number to determine if it is a power of two.
 * \return bool_e
 * \retval true_e It is a power of two.
 * \retval false_e It is not a power of two.
 */
static bool_e is_power_two(size_t a)
{
    value_t ones = 0;
    uint32_t bit = 0;
    for (bit = 0; bit < (8 * sizeof(value_t)); bit++)
    {
        if (((1 << bit) & a) != 0)
            ones++;
    }
    if (ones > 1 || ones == 0)
        return false_e;
    else
        return true_e;
}
#endif

fph_t *fph_init(size_t numBlocks, size_t blockSize, fph_key_f keyFunc)
{
    fph_t *fph = (fph_t*)malloc(sizeof(fph_t));
    if (fph)
    {
        memset(fph, 0, sizeof(fph_t));
#ifdef FPH_OPTIMIZED
        if (!is_power_two(numBlocks))
        {
            free(fph);
            return NULL;
        }
#endif
        fph->numBlocks = numBlocks;
        fph->blockSize = blockSize;
        fph->numElem = fph->numBlocks * fph->blockSize;
        if (keyFunc)
            fph->keyFunc = keyFunc;
        else
            fph->keyFunc = fph_hash;
        fph->hash = (fph_kv_t *)malloc(fph->numElem * sizeof(fph_kv_t));
        if (fph->hash)
            memset(fph->hash, 0, fph->numElem * sizeof(fph_kv_t));
#ifdef FPH_METRICS
        fph->collisions = (uint32_t *)malloc(fph->numBlocks * sizeof(uint32_t));
        if (fph->collisions)
            memset(fph->collisions, 0, fph->numBlocks * sizeof(uint32_t));
        fph->length = (uint32_t *)malloc(fph->numBlocks * sizeof(uint32_t));
        if (fph->length)
            memset(fph->length, 0, fph->numBlocks * sizeof(uint32_t));
#endif
    }
    return fph;
}

void fph_deinit(fph_t *fph)
{
    if (fph)
    {
        if (fph->hash)
            free(fph->hash);
#ifdef FPH_METRICS
        if (fph->collisions)
            free(fph->collisions);
        if (fph->length)
            free(fph->length);
#endif
        memset(fph, 0xEF, sizeof(fph_t));
        free(fph);
    }
    return;
}

int fph_get(fph_t *fph, ptr_t k, ptr_t *v)
{
    uint32_t i = 0;
    uint32_t index = fph->keyFunc(fph, k);
    *v = NULL;
    for (i = index; i < (index + fph->blockSize); i++) {
        if (fph->hash[i].key == k) {
            *v = fph->hash[i].value;
            return 1;
        }
    }
    return 0;
}

int fph_clr(fph_t *fph, ptr_t k)
{
    uint32_t i = 0;
    uint32_t block, index = fph->keyFunc(fph,k);
    for (i = index; i < (index + fph->blockSize); i++) {
        if (fph->hash[i].key == k) {
            fph->hash[i].key = NULL;
            fph->hash[i].value = NULL;
#ifdef FPH_METRICS
            block = index / fph->blockSize;
            if (fph->length[block] > 1)
                fph->collisions[block]--;
            else if (fph->length[block] == 1)
                fph->collisions[block] = 0;
            fph->length[block]--;
            fph->numPtrs--;
#endif
            return 1;
        }
    }
    return 0;
}

int fph_set(fph_t *fph, ptr_t k, ptr_t v)
{
    uint32_t index, i, block;

    if (fph == NULL || k == NULL) // this is not valid
        return 0;

    // determine the index
    index = fph->keyFunc(fph, k);
#ifdef FPH_METRICS
    block = index / fph->blockSize;
#endif
    for (i = index; i < (index + fph->blockSize); i++) {
        if (fph->hash[i].key == k) { // REPLACEMENT
            fph->hash[i].key = k;
            fph->hash[i].value = v;
#ifdef FPH_METRICS
            fph->numReplacements++;
            fph->collisions[block]++;
#endif
            return 1;
        }
        if (fph->hash[i].key == NULL) { // NEW FILL
            fph->hash[i].key = k;
            fph->hash[i].value = v;
#ifdef FPH_METRICS
            fph->length[block]++;
            if (fph->length[block] > fph->blockSize)
            {
                SOSAL_PRINT(SOSAL_ZONE_HASH, "ERROR! BlockSize has been exceeded!\n");
            }
            fph->numPtrs++;
#endif
            return 1;
        }
    }
#ifdef FPH_METRICS
    if (i == (index + fph->blockSize))
    {
        SOSAL_PRINT(SOSAL_ZONE_WARNING, "WARNING!! Fell out of hash!\n");
        fph->numMisses++;
    }
#endif
    return 0;
}

#ifdef FPH_TEST
#include <math.h>
#include <time.h>

ptr_t rand_ptr()
{
    uint32_t range = 0xFFFFFFFF;
    int r = rand();
    ptr_t p = 0;

    if (range > RAND_MAX)
        r <<= 8;
    p = (ptr_t)r;
    return p;
}

typedef struct _stats_t {
    uint32_t sum;
    uint32_t avg;
    uint32_t cnt;
} Stats_t;

#define TWO_POW(x)  (1<<(x))

int main(int argc, char *argv[])
{
    uint32_t n = (argc > 1 ? atoi(argv[1]) : 15);
    uint32_t numPtrs = TWO_POW(n);
    rtime_t c_start, c_linear, c_hash;
    rtime_t c_lsum = 0, c_hsum = 0;
    double c_lavg, c_havg;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    srand((uint32_t)time(NULL));

    fph_t *fph = fph_init(TWO_POW(n/2 + 1), TWO_POW(n/2 + 1), NULL);
    fph_kv_t *linear = (fph_kv_t *)malloc(numPtrs * sizeof(fph_kv_t));

    if (fph && linear)
    {
        uint32_t i,j;
        ptr_t k,v;
        Stats_t collisions;
        Stats_t lengths;

        memset(linear, 0, sizeof(fph_kv_t) * numPtrs);
        // create bunch of pointers and add them to the hash
        for (i = 0; i < numPtrs; i++)
        {
            // generate a random value pair
            k = rand_ptr();
            v = rand_ptr();

            // add this to the linear array and the hash
            c_start = rtimer_now();
            for (j = 0; j < numPtrs; j++) {
                if (linear[j].key == NULL && linear[j].value == NULL) {
                    linear[j].key = k;
                    linear[j].value = v;
                }
            }
            c_linear = rtime_now() - c_start;
            c_start = rtime_now();
            fph_set(fph, k, v);
            c_hash = rtime_now() - c_start;

            c_hsum += c_hash;
            c_lsum += c_linear;
        }
        c_lavg = (double)c_lsum / numPtrs;
        c_havg = (double)c_hsum / numPtrs;

        memset(&collisions, 0, sizeof(Stats_t));
        memset(&lengths, 0, sizeof(Stats_t));

#ifdef FPH_METRICS
        // collate data
        for (i = 0; i < fph->numBlocks; i++)
        {
            //SOSAL_PRINT(SOSAL_ZONE_HASH, "[%u] collisions = %u, length = %u\n", i, fph->collisions[i], fph->length[i]);
            lengths.sum += fph->length[i];
            collisions.sum += fph->collisions[i];
        }
        collisions.cnt = fph->numBlocks;
        lengths.cnt = fph->numBlocks;
        collisions.avg = collisions.sum / collisions.cnt;
        lengths.avg = lengths.sum / lengths.cnt;
#endif

        SOSAL_PRINT(SOSAL_ZONE_HASH, "Num Blocks: %u\n", fph->numBlocks);
        SOSAL_PRINT(SOSAL_ZONE_HASH, "Block Size: %u\n", fph->blockSize);
#ifdef FPH_METRICS
        SOSAL_PRINT(SOSAL_ZONE_HASH, "Collisions: Sum %u :: Avg %lu\n", collisions.sum, collisions.avg);
        SOSAL_PRINT(SOSAL_ZONE_HASH, "Lengths: Sum %u :: Avg %lu\n", lengths.sum, lengths.avg);
        SOSAL_PRINT(SOSAL_ZONE_HASH, "Misses: %u\n", fph->numMisses);
        SOSAL_PRINT(SOSAL_ZONE_HASH, "Total Hash Ptrs: %u\n", fph->numPtrs);
        SOSAL_PRINT(SOSAL_ZONE_HASH, "Replacements: %u\n",fph->numReplacements);

#endif
        SOSAL_PRINT(SOSAL_ZONE_HASH, "Linear Time: Sum %lu :: Avg %lf\n", c_lsum, c_lavg);
        SOSAL_PRINT(SOSAL_ZONE_HASH, "Hash Time: Sum %lu :: Avg %lf\n", c_hsum, c_havg);
        SOSAL_PRINT(SOSAL_ZONE_HASH, "rtimer_freq()="FMT_RTIMER_T"\n",rtimer_freq());

        free(linear);
        fph_deinit(fph);
    }
    else
    {
        if( linear ) {
            free(linear);
            linear = NULL;
        }

        if( fph ) {
            fph_deinit(fph);
        }
    }

    return 0;
}
#endif
