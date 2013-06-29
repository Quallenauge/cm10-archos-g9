/**
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

#include <sosal/cache.h>
#include <sosal/module.h> // just for unittest
#include <sosal/debug.h>

void cache_destroy(cache_t *cache)
{
    size_t i = 0;
    for (i = 0; i < cache->numLines; i++)
    {
        if (cache->line[i].data != NULL)
        {
            if (cache->line[i].data)
                free(cache->line[i].data);
            cache->line[i].data = NULL;
        }
    }
    free(cache->line);
    mutex_deinit(&cache->mutex);
    memset(cache, 0, sizeof(cache_t));
    free(cache);
}

cache_t *cache_create(size_t lineSize,
                      size_t numLines,
                      cache_policy_e policy,
                      cache_fill_f fill,
                      cache_commit_f commit,
                      void *cookie)
{
    cache_t *cache = (cache_t *)calloc(1, sizeof(cache_t));
    if (cache)
    {
        size_t i = 0;
        cache->numLines = numLines;
        cache->lineSize = lineSize;
        cache->cookie = cookie;
        cache->fillLine = fill;
        cache->commitLine = commit;
        cache->policy = policy;
        mutex_init(&cache->mutex);
        cache->line  = calloc(cache->numLines, sizeof(cache_line_t));
        if (cache->line)
        {
            for (i = 0; i < cache->numLines; i++)
            {
                cache->line[i].data = calloc(cache->lineSize, 1);
                if (cache->line[i].data == NULL)
                {
                    cache_destroy(cache);
                    cache = NULL;
                    break;
                }
            }
        }
        else
        {
            cache_destroy(cache);
            cache = NULL;
        }
    }
    return cache;
}

bool_e cache_index(cache_t *cache, size_t offset, size_t *index)
{
    size_t i;
    for (i = 0; i < cache->numLines; i++)
    {
        if (cache->line[i].valid == true_e && cache->line[i].start_offset <= offset && offset < cache->line[i].end_offset)
        {
            *index = i;
            return true_e;
        }
    }
    return false_e;
}

static size_t cache_round_offset(cache_t *c, size_t offset)
{
    return (offset / c->lineSize) *  c->lineSize;
}

static size_t cache_next(cache_t *cache)
{
    size_t i = 0;
    bool_e found = false_e;
    size_t free_index = 0;
    size_t oldest_index = 0;
    clock_t oldest_clock = clock();
    // find the an empty cache line
    for (i = 0; i < cache->numLines; i++)
    {
        if (cache->line[i].valid == false_e) {
            free_index = i;
            found = true_e;
        } else {
            if (cache->line[i].locked == false_e && cache->line[i].age < oldest_clock) {
                oldest_clock = cache->line[i].age;
                oldest_index = i;
            }
        }
    }
    if (found)
        return free_index;
    else {
        // age out the oldest!
        if (cache->policy == CACHE_POLICY_WRITE_BACK)
            cache->commitLine(cache->cookie, cache->line[i].data, cache->lineSize, cache->line[i].start_offset);
        cache->line[i].valid = false_e;
        return oldest_index;
    }
}

size_t cache_read(cache_t *cache, uint8_t *data, size_t len, size_t offset)
{
    size_t bytesRead = 0;

    // lock the cache
    mutex_lock(&cache->mutex);
    do
    {
        size_t index = 0;
        size_t intraOffset = (offset % cache->lineSize);

        // calculate the useful size within this line.
        size_t size = cache->lineSize - intraOffset;
        if (len < cache->lineSize ) // if the remainder is smaller than the lineSize
            size = len; // then use the remainder

        // get the line index for this ptr, if it exists
        if (cache_index(cache, offset, &index) == false_e)
        {
            // find a cache line to fill
            index = cache_next(cache);

            // fetch the data and assign the index
            cache->fillLine(cache->cookie, cache->line[index].data, cache->lineSize, cache_round_offset(cache, offset));
        }

        // copy the data from this line...
        memcpy(data, &cache->line[index].data[intraOffset], size);

        // move the pointers
        data += size;
        offset += size;
        len -= size;
        bytesRead += size;
    } while(len > 0);
    // unlock the cache
    mutex_unlock(&cache->mutex);
    return bytesRead;
}

size_t cache_write(cache_t *cache, uint8_t *data, size_t len, size_t offset)
{
    size_t written = 0;

    // lock the cache
    mutex_lock(&cache->mutex);
    do
    {
        size_t index = 0;
        size_t intraOffset = (offset % cache->lineSize);

        // calculate the useful size within this line.
        size_t size = cache->lineSize - intraOffset;
        if (len < cache->lineSize) // if the remainder is smaller than the lineSize
            size = len; // then use the remainder

        // get the line index for this ptr, if it exists
        if (cache_index(cache, offset, &index) == false_e)
        {
            // find a cache line to fill
            // this doesn't fail, if there are no open ones it will age out an
            // older line and we'll need to initialize it's data
            index = cache_next(cache);

            // initialize the correct start_offset and size
            cache->line[index].start_offset = cache_round_offset(cache, offset);
            cache->line[index].end_offset = cache->line[index].start_offset + cache->lineSize;
        }

        // copy the data from this line...
        memcpy(&cache->line[index].data[intraOffset], data, size);

        // update the meta-data
        cache->line[index].valid = true_e;
        cache->line[index].age = clock();

        // if it's a write-through policy, go ahead and write
        if (cache->policy == CACHE_POLICY_WRITE_THROUGH)
            cache->commitLine(cache->cookie, cache->line[index].data, cache->lineSize, cache->line[index].start_offset);

        // move the pointers
        data += size;
        offset += size;
        len -= size;
        written += size;
        //printf("%u bytes left to write to cache\n", len);
    } while(len > 0);
    // unlock the cache
    mutex_unlock(&cache->mutex);
    return written;
}

void cache_invalidate(cache_t *cache)
{
    size_t i = 0;
    mutex_lock(&cache->mutex);
    for (i = 0; i < cache->numLines; i++)
    {
        if (cache->line[i].valid)
        {
            cache->line[i].age = 0;
            cache->line[i].valid = false_e;
        }
    }
    mutex_unlock(&cache->mutex);
}

void cache_flush(cache_t *cache)
{
    size_t i = 0;
    mutex_lock(&cache->mutex);
    for (i = 0; i < cache->numLines; i++)
    {
        if (cache->line[i].valid)
        {
            cache->commitLine(cache->cookie, cache->line[i].data, cache->lineSize, cache->line[i].start_offset);
            cache->line[i].age = 0;
            cache->line[i].valid = false_e;
        }
    }
    mutex_unlock(&cache->mutex);
}

//******************************************************************************
// INTERNAL TESTING
//******************************************************************************

static size_t cache_fill(void *cookie, uint8_t *ptr, size_t size, size_t offset)
{
    FILE *f = (FILE *)cookie;
    //fpos_t foffset = (fpos_t)offset;
    //fsetpos(f, &foffset);
    fseek(f, offset, SEEK_SET);
    //printf("Filling Cache Line @ %lu for %lu\n", offset, size);
    return fread(ptr, 1, size, f);
}

static size_t cache_commit(void *cookie, uint8_t *ptr, size_t size, size_t offset)
{
    FILE *f = (FILE *)cookie;
    //fpos_t foffset = (fpos_t)offset;
    //fsetpos(f, &foffset);
    fseek(f, offset, SEEK_SET);
    //printf("Committing Cache Line @ %lu for %lu\n", offset, size);
    return fwrite(ptr, 1, size, f);
}

static void countUp(uint8_t *ptr, size_t size, uint32_t start)
{
    size_t i = 0;
    uint32_t *p = (uint32_t *)ptr;
    for (i = 0; i < (size/sizeof(uint32_t)); i++)
    {
        p[i] = i + start;
    }
}

bool_e cache_unittest(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    bool_e ret = true_e;
    size_t i;
    size_t numBufs  = (1<<10);
    size_t bufSize  = (1<<8);
    size_t lineSize = (1<<7);
    size_t numLines = (1<<11);
    size_t offset = 0;
    uint8_t *buffer = (uint8_t *)calloc(bufSize, sizeof(uint8_t));
    FILE *f = fopen("cache.dat", "w+");
    SOSAL_PRINT(SOSAL_ZONE_API, "cache_unittest(%u, %p)\n", argc, argv);
    if (f != NULL)
    {
        cache_t *c = cache_create(lineSize, numLines, CACHE_POLICY_WRITE_THROUGH, cache_fill, cache_commit, f);
        if (c)
        {
            // fill up the cache!
            //printf("Created cache!\n");
            for (i = 0; i < numBufs; i++)
            {
                countUp(buffer, bufSize,i*bufSize);
                cache_write(c, buffer, bufSize, offset);
                offset += bufSize;
            }
            //printf("Fill up cache items\n");

            // commit everything to the file
            cache_flush(c);

            // read back the data and verify it
            offset = 0;
            for (i = 0; i < numBufs; i++)
            {
                if (cache_read(c, buffer, bufSize, offset) == bufSize)
                {
                    uint32_t *p = (uint32_t *)buffer;
                    size_t j = 0;
                    for (j = 0; j < bufSize/sizeof(uint32_t); j++)
                    {
                        if (p[j] != j + (i*bufSize))
                        {
                            printf("ERROR! Failed count check on item %u!="FMT_SIZE_T"\n", p[j], j+(i*bufSize));
                            ret = false_e;
                        }
                    }
                    offset += bufSize;
                }
                else
                {
                    printf("ERROR! Failed to read enough bytes!\n");
                }
            }

            // toss everything that's in the cache
            cache_invalidate(c);

            cache_destroy(c);
            c = NULL;
        }
        else
        {
            printf("ERROR: Failed to create cache!\n");
            ret = false_e;
        }
        fclose(f);
        f = NULL;
    }
    else
    {
        printf("ERROR: Failed to open cache file, probably a permission issue or read-only location?\n");
        ret = false_e;
    }
    f = fopen("poetry.txt","r");
    if (f)
    {
        size_t len = 0;
        fseek(f, 0, SEEK_END);
        len = ftell(f);
        //printf("File %p is %s is %lu bytes long\n", f, "test"PATH_DELIM"poetry.txt", len);

        if (len > 0)
        {
            cache_t *c = cache_create(lineSize, numLines, CACHE_POLICY_WRITE_THROUGH, cache_fill, cache_commit, f);
            if (c)
            {
                const size_t compSize = 200;
                uint32_t i,iterations = 10;
                uint8_t *cached = malloc(compSize);
                uint8_t *uncached = malloc(compSize);

                // read an arbitrary offset in to the file for an arbitrary size
                // and compare with what the cache sees
                for (i = 0; i < iterations; i++)
                {
                    offset = rand() % (len-compSize);
                    cache_read(c, cached, compSize, offset);
                    fseek(f, offset, SEEK_SET);
                    if (fread(uncached, 1, compSize, f) > 0)
                    {
                        //printf("Comparing %lu bytes @ %lu offset\n", compSize, offset);
                        if (memcmp(cached, uncached, compSize) != 0)
                        {
                            printf("Failed seek to "FMT_SIZE_T"!\n", offset);
                            ret = false_e;
                        }
                    }
                }
                free(cached);
                free(uncached);
                cache_destroy(c);
            }
            else
                ret = false_e;
        }
        else
            ret = false_e;
        fclose(f);
    }
    //else
    //    ret = false_e;
    return ret;
}


