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

#include <sosal/hash.h>
#include <sosal/options.h>
#include <sosal/debug.h>

static record_t *record_create(value_t key, value_t value)
{
    record_t *record = calloc(1, sizeof(record_t));
    if (record)
    {
        record->key = key;
        record->value = value;
    }
    return record;
}

static void record_destroy(record_t *record)
{
    if (record)
        free(record);
}

static int hash_compare_record(node_t *a, node_t *b)
{
    if (a != NULL && b != NULL)
    {
        record_t *ra = (record_t *)a->data;
        record_t *rb = (record_t *)b->data;
        if (ra != NULL && rb != NULL)
        {
            if (ra->key > rb->key)
                return 1;
            else if (ra->key == rb->key)
                return 0;
            else if (ra->key < rb->key)
                return -1;
        }
    }
    return 0;
}

static void hash_print_record(node_t *node)
{
    if (node)
    {
        record_t *record = (record_t *)node->data;

        if (record != NULL) {
            SOSAL_PRINT(SOSAL_ZONE_HASH, ""FMT_SIZE_T":"FMT_SIZE_T" ",record->key, record->value);
        }
    }
}

static int hash_is_valid_index(hash_t *h, value_t hKey)
{
    if ((h->size == HASH_SIZE_SMALL ||
         h->size == HASH_SIZE_MEDIUM ||
         h->size == HASH_SIZE_LARGE) &&
        (hKey < h->numSlots))
        return 1;
    else
        return 0;
}

static uint32_t hash_key_to_index(hash_t *h, value_t hKey)
{
    if ((h->size == HASH_SIZE_SMALL ||
         h->size == HASH_SIZE_MEDIUM ||
         h->size == HASH_SIZE_LARGE))
        return (uint32_t)hKey;
    else
        return 0;
}

/**
 * This is the default keying function.
 */
static value_t hash_key(size_t numElem, value_t keyin)
{
    value_t key = keyin;
    uint32_t mask = 0;

    key += (key << 10);
    key ^= (key >> 6);
    key += (key << 3);
    key ^= (key >> 11);
    key += (key << 15);

    mask = (1 << numElem) - 1;
    key &= mask;

    //SOSAL_PRINT(SOSAL_ZONE_HASH, "Hash Function: %d => %lu\n", keyin, key);

    return key;
}

/* =============================================================================
 * GLOBAL FUNCTIONS
 * ===========================================================================*/


void hash_destroy(hash_t *h)
{
    if (h != NULL)
    {
        if (h->bucket != NULL)
        {
            free(h->bucket);
        }
        free(h);
    }
}

hash_t *hash_create(hash_size_e size, hash_func_f keyFunc)
{
    hash_t *h = NULL;
    int hashElem = 1 << size;

    h = (hash_t *)calloc(1, sizeof(hash_t));
    if (h != NULL)
    {
        h->bucket = (list_t **)calloc(hashElem, sizeof(list_t *));
        if (h->bucket == NULL)
        {
            free(h);
            h = NULL;
        }
        else
        {
            h->numSlots = hashElem;
            h->numActiveSlots = 0;
            h->numTotalElem = 0;
            h->size = size;
            if (keyFunc == NULL)
                h->keyFunc = hash_key;
            else
                h->keyFunc = keyFunc;
        }
    }
    return h;
}



void hash_set(hash_t *h, value_t key, value_t value)
{
    // is the hash valid?
    if (h != NULL)
    {
        // generate the key
        value_t hKey = h->keyFunc(h->size, key);

        // is the key valid?
        if (hash_is_valid_index(h, hKey))
        {
            uint32_t  index = hash_key_to_index(h,hKey);
            record_t *record = record_create(key, value);
            node_t   *node = node_create((value_t)record);

            // do we have a collision?
            if (h->bucket[index] == NULL && value != 0)
            {
                // no, create a list and the first element
                //SOSAL_PRINT(SOSAL_ZONE_HASH, "Created new list added node to list\n");
                hash_print_record(node);
                h->bucket[index] = list_create();
                list_push(h->bucket[index], node);
                h->numActiveSlots++;
                h->numTotalElem++;
                SOSAL_PRINT(SOSAL_ZONE_HASH, "SET: "FMT_SIZE_T":"FMT_SIZE_T"\n", record->key, record->value);
            }
            else if (h->bucket[index] != NULL)
            {
                // there's a key collision
                //SOSAL_PRINT(SOSAL_ZONE_HASH, "Collision detected!\n");

                // are we removing the value or adding a value?
                if (value == 0)
                {
                    node_t *removed = NULL;
                    do {
                        removed = list_remove_match(h->bucket[index], node, hash_compare_record);
                        if (removed != NULL)
                        {
#if defined(SOSAL_DEBUG)
                            record_t *r = (record_t *)removed->data;
#endif
                            //SOSAL_PRINT(SOSAL_ZONE_HASH, "Removed Record from List!\n");
                            h->numTotalElem--;
                            if (list_length(h->bucket[index]) == 0)
                            {
                                list_destroy(h->bucket[index]);
                                h->numActiveSlots--;
                                h->bucket[index] = NULL;
                            }
                            SOSAL_PRINT(SOSAL_ZONE_HASH, "CLR: "FMT_SIZE_T":"FMT_SIZE_T"\n", r->key, r->value);
                            record_destroy((record_t*)node_destroy(removed));
                        }
                    } while (removed != NULL);
                }
                else // we're adding to a link list
                {
                    // remove existing key/value pair if it exists
                    node_t *removed = list_remove_match(h->bucket[index], node, hash_compare_record);
                    if (removed)
                    {
#if defined(SOSAL_DEBUG)
                        record_t *r = (record_t *)removed->data;
#endif
                        //SOSAL_PRINT(SOSAL_ZONE_HASH, "Record already exists in hash, removing!\n");
                        h->numTotalElem--;
                        SOSAL_PRINT(SOSAL_ZONE_HASH, "CLR: "FMT_SIZE_T":"FMT_SIZE_T"\n", r->key, r->value);
                        record_destroy((record_t*)node_destroy(removed));
                        // don't remove the list here, we're about to add to it.
                    }

                    //SOSAL_PRINT(SOSAL_ZONE_HASH, "Adding record to hash...\n");
                    // put the items in sorted order
                    node = list_insert(h->bucket[index], node, hash_compare_record, true_e);
                    if (node)
                    {
                        h->numTotalElem++;
                        SOSAL_PRINT(SOSAL_ZONE_HASH, "SET: "FMT_SIZE_T":"FMT_SIZE_T"\n", record->key, record->value);
                    }
                    else
                    {
                        SOSAL_PRINT(SOSAL_ZONE_HASH, "SET: "FMT_SIZE_T":--\n", record->key);
                    }
                }
            }
            else if (h->bucket[index] == NULL && value == 0)
            {
                // tried to remove something that wasn't there.
                record_destroy((record_t *)node_destroy(node));
                return;
            }
        }
    }
    return;
}

bool_e hash_get(hash_t *h, value_t key, value_t *value)
{
    bool_e found = false_e;

    // is the hash valid and does it have items?
    if (h != NULL && h->numActiveSlots > 0)
    {
        // generate the key
        value_t hKey = h->keyFunc(h->size, key);

        // is the key valid?
        if (hash_is_valid_index(h, hKey))
        {
            // define tmp pointer
            uint32_t index = hash_key_to_index(h,hKey);

            // is there something at that index?
            if (h->bucket[index] != NULL)
            {
                record_t *record = record_create(key, 0);
                node_t *node = node_create((value_t)record);
                node_t *desired = NULL;

                // now get value from the list
                desired = list_search(h->bucket[index], node, hash_compare_record);

                // key was found in the list
                if (desired != NULL)
                {
                    record_t *match = (record_t *)desired->data;
                    SOSAL_PRINT(SOSAL_ZONE_HASH, "GET: "FMT_SIZE_T":"FMT_SIZE_T"\n", match->key, match->value);

                    // get the value
                    *value = match->value;
                    found = true_e;
                }
                node_destroy(node);
                record_destroy(record);
            }
        }
    }
    if (found == false_e) {
        SOSAL_PRINT(SOSAL_ZONE_HASH, "GET: "FMT_SIZE_T":---\n", key);
    }
    return found;
}

/**
 * @note If the hash index loops you will get the same count back again and
 *       your pointer will not be set. Be sure to NULL (and check) your pointer
 *       between each usage to prevent this scenario.
 */
size_t hash_clean(hash_t *h, value_t *value)
{
    static uint32_t index = 0;

    if (h != NULL && value != NULL && h->numActiveSlots > 0)
    {
        // move to a non-null section.
        while (h->bucket[index] == NULL && index < h->numSlots) {
            //SOSAL_PRINT(SOSAL_ZONE_HASH, "Bucket[%u] is empty\n", index);
            index++;
        }

        if (index >= h->numSlots)
        {
            // reset the index
            index = 0;
        }

        // if we are in hash still...
        if (index < h->numSlots)
        {
            // we found a list
            list_t *bucket = h->bucket[index];
            node_t *node = list_pop(bucket);
            if (node)
            {
                record_t *record = (record_t *)node->data;
                h->numTotalElem--;
                *value = record->value;
                if (list_length(bucket) == 0)
                {
                    list_destroy(bucket);
                    h->numActiveSlots--;
                    h->bucket[index] = NULL;
                }
            }
        }
        return h->numTotalElem;
    }
    else
        return 0;
}

size_t hash_length(hash_t *h)
{
    size_t length = 0;
    if (h != NULL)
    {
        length = h->numTotalElem;
    }
    return length;
}

void hash_print(hash_t *h, bool_e printElems)
{
    unsigned int i;
    int sum = 0;
    int avg = 0;
    int num = 0;
    int len = 0;
    float util = 0.0;
    if (h != NULL)
    {
        util = (float)((float)h->numActiveSlots * 100)/h->numSlots;
        printf("[%p] hash_t ["FMT_SIZE_T"/"FMT_SIZE_T"] => %lf%%\n", h, h->numActiveSlots, h->numSlots, util);
        if (printElems == true_e)
        {
            for (i = 0; i < h->numSlots;i++)
            {
                len = list_length(h->bucket[i]);
                if (len > 0)
                {
                    printf("Bucket[%u] has %u records: ",i,len);
                    sum += len;
                    num++;
                    avg = sum/num;
                    list_print(h->bucket[i], hash_print_record);
                    printf("\n");
                }
            }
            printf("Average List Depth is %d\n",avg);
        }
        printf("\tTotal hash_t Size is "FMT_SIZE_T" elements\n",h->numTotalElem);
        printf("\tTotal Collisions is "FMT_SIZE_T"\n",(h->numTotalElem - h->numActiveSlots));
    }
}

static value_t rand_value()
{
    return (value_t)(rand()<<2);
}

bool_e hash_unittest(int argc, char *argv[])
{
    int numElem = 0;
    value_t getValue = 0;
    value_t setValue = 0;
    value_t tmpValue = 0;
    value_t key = 0;
    int i = 0;
    int numErrors = 0;
    hash_size_e size = HASH_SIZE_SMALL;
    bool_e verbose = false_e;
    option_t hash_opts[] = {
        {OPTION_TYPE_BOOL, &verbose, sizeof(bool_e), "-v", "--verbose", "Used to print out debugging information"},
        {OPTION_TYPE_INT, &size, sizeof(int), "-hs", "--hash-size", "Used to indicate the hash size"},
    };
    hash_t *h = NULL;

    option_process(argc, argv, hash_opts, 1);

    h = hash_create(size, NULL);
    numElem = (1 << (h->size-1)); // half the elements that we could hold...
    if (verbose)
        printf("Adding %u elements to hash\n",numElem);
    for (i = 0; i < numElem; i++)
    {
        key = rand();
        setValue = rand_value();

        // set the initial value...
        hash_set(h, key, setValue);

        // now remove it
        hash_set(h, key, 0);

        // was it there?
        if (hash_get(h, key, &getValue) == true_e)
        {
            numErrors++;
            if (verbose)
                printf("Found a stale value in hash!\n");
        }

        // re-add the value
        hash_set(h, key, setValue);

        // now overwrite it
        tmpValue = rand_value();
        hash_set(h, key, tmpValue);

        if (hash_get(h, key, &getValue) && setValue == getValue)
        {
            numErrors++;
            printf("WARNING! Found stale value in hash, should have been overwritten!\n");
        }

        // re-overwrite to final value
        hash_set(h, key, setValue);

        // re-get it
        if (hash_get(h, key, &getValue) == true_e)
        {
            if (setValue != getValue)
            {
                numErrors++;
                printf("ERROR: Setting "FMT_SIZE_T" returned "FMT_SIZE_T" on get\n",setValue,getValue);
            }
            else if (verbose)
            {
                printf("[%u] Set "FMT_SIZE_T" correctly!\n",i,setValue);
            }
        }
        else
        {
            numErrors++;
            printf("ERROR: Failed to find known value!\n");
        }
    }

    if (verbose)
        hash_print(h, true_e);
    do {
        numElem = hash_clean(h, &getValue);
        //if (verbose)
            //printf("cleaned out %p (%d left)\n", getValue, numElem);
    } while (numElem > 0);

    if (hash_length(h) != 0)
    {
        numErrors++;
        printf("ERROR! hash was not cleaned!\n");
    }

    if (verbose)
        hash_print(h, false_e);
    hash_destroy(h);

    if (numErrors > 0) {
        printf("Hashing Unit Test failed with %u errors!\n", numErrors);
        return false_e;
    } else {
        return true_e;
    }
}

