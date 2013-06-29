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

#ifndef _SOSAL_CACHE_H_
#define _SOSAL_CACHE_H_

/*!
 * \file cache.h
 * \brief A Memory Caching Structure which can be used in front of a frequently
 * read file or any high latency access data structure.
 * \author Erik Rainey
 */

#include <sosal/types.h>
#include <sosal/mutex.h>

/*! \brief This function backs the cache with it's data source at the appropriate index.
 * \ingroup group_cache
 */
typedef size_t (*cache_fill_f)(void *cookie, uint8_t *ptr, size_t size, size_t offset);

/*! \brief This function writes a line back to the data source from the specified index
 * \ingroup group_cache
 */
typedef size_t (*cache_commit_f)(void *cookie, uint8_t *ptr, size_t size, size_t offset);

/*! \brief These policies change the default behavior of the cache.
 * \ingroup group_cache
 */
typedef enum _cache_policy_e {
    CACHE_POLICY_WRITE_THROUGH, /*!< Data is committed when written. */
    CACHE_POLICY_WRITE_BACK,	/*!< Data is committed when aged out. */
} cache_policy_e;

/*! \brief The line data of the cache.
 * \ingroup group_cache
 */
typedef struct _cache_line_t {
    bool_e locked;
    bool_e valid;
    clock_t age;
    size_t start_offset;
    size_t end_offset;
    uint8_t  *data;
} cache_line_t;

/*! \brief The top level structure of the cache.
 * \ingroup group_cache
 */
typedef struct _cache_t {
    cache_line_t  *line;		/*!< The pointer to the line data. */
    size_t         numLines;	/*!< The number of lines in the cache. */
    size_t         lineSize;	/*!< The size of each line in bytes. */
    cache_policy_e policy;		/*!< The aging policy. */
    cache_fill_f   fillLine;	/*!< The line filling function. */
    cache_commit_f commitLine;	/*!< The line committing function. */
    void          *cookie;		/*!< The user's private data to pass to the functions. */
    mutex_t        mutex;		/*!< The mutex which protects data access for the cache. */
} cache_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This function destroys a cache. All the lines will be committed or flushed
 * before destruction.
 * \param [in] cache The pointer to the cache metadata.
 * \ingroup group_cache
 * \pre \ref cache_create
 */
void cache_destroy(cache_t *cache);

/*! \brief This method creates an in-memory cache for use with external high-latency
 * sources. For example, this could be placed in front of a file or socket.
 * \param [in] lineSize The size of each line in bytes.
 * \param [in] numLines The number of lines in the cache.
 * \param [in] policy The cache aging policy.
 * \param [in] fill The cache filling function.
 * \param [in] commit The line committing function.
 * \param [in] cookie The private user data to pass into the supplied functions.
 * \ingroup group_cache
 */
cache_t *cache_create(size_t lineSize,
                      size_t numLines,
                      cache_policy_e policy,
                      cache_fill_f fill,
                      cache_commit_f commit,
                      void *cookie);

/*! \brief This functions reads data through the cache from the source.
 * \param [in] cache The pointer to the cache metadata.
 * \param [in] data The pointer to the buffer to read data into.
 * \param [in] len The length of the data to read.
 * \param [in] offset The offset in the cache to read from.
 * \ingroup group_cache
 */
size_t cache_read(cache_t *cache, uint8_t *data, size_t len, size_t offset);

/*! \brief This function writes data through the cache into the source.
 * \param [in] cache The pointer to the cache metadata.
 * \param [in] data The pointer to the buffer to write data from into the cache.
 * \param [in] len The length of the data to write.
 * \param [in] offset The offset in te cache to write the data to.
 * \ingroup group_cache
 */
size_t cache_write(cache_t *cache, uint8_t *data, size_t len, size_t offset);

#ifdef __cplusplus
}
#endif

#endif

