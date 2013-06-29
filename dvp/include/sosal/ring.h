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

#ifndef _SOSAL_RING_H_
#define _SOSAL_RING_H_

/*! \file  
 * \brief The SOSAL Ring Buffer API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>

/*! \brief The ring buffer data structure.
 * \ingroup group_rings
 */
typedef struct _ring_t{
    size_t totalNumBytes;  /*!< Total number of bytes in a ring buffer. */
    size_t numBytesUsed;   /*!< The number of bytes filled with data */
    size_t numBytesFree;   /*!< The number of empty bytes */
    uint8_t *start;        /*!< alwars points to the first item in the linear array */
    uint8_t *end;          /*!< always points to the last item in the linear array */
    uint8_t *front;        /*!< always points to the first item in the ring */
    uint8_t *back;         /*!< always points to the last item in the ring */
} ring_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a ring buffer
 * This function allocates a ring buffer and it's control structure as well
 * as initializing it's control structure's member variables.
 * \param [in] totalSize This describes the total number of bytes to allocate
 *                      for the ring buffer.
 * \return Returns a pointer to the control structure on success or NULL
 *         on failure.
 * \post \ref ring_destroy
 * \ingroup group_rings
 */
ring_t *ring_create(size_t totalSize);

/**
 * \brief This function destroy's the ring buffer and it's control structure.
 * \param [out] rb The pointer to the ring buffer's control structure.
 * \pre \ref ring_create
 * \ingroup group_rings
 */
void ring_destroy(ring_t *rb);

/**
 * \brief Write len bytes of data to a ring buffer rb
 *
 * \param [in,out] rb  the ring buffer to write data to.
 * \param [in]     b   pointer to the data to be written.
 * \param [in]     len The desired length to write.
 * \return Returns the number of bytes written to the ring buffer. If zero,
 *         no bytes were written.
 * \pre \ref ring_create
 * \ingroup group_rings
 */
size_t ring_write(ring_t *rb, uint8_t *b, size_t len);

/**
 * \brief Read len bytes of data from a ring buffer rb
 * \param [in,out] rb  the ring buffer to read data from.
 * \param [out]    b   pointer to the output data buffer.
 * \param [in]     len The desired length read, can be considered the max len read
 * \return Returns the number of bytes read from the ring buffer. If zero,
 *         no bytes were read.
 * \pre \ref ring_create
 * \ingroup group_rings
 */
size_t ring_read(ring_t *rb, uint8_t *b, size_t len);

/**
 * This function prints the values of the control structure as well as the
 * values of the ring buffer as well. Do not use on large buffers!
 * \param [in] rb The pointer to the control structure of the ring buffer
 * \pre \ref ring_create
 * \ingroup group_rings
 */
void ring_print(ring_t *rb);


#ifdef __cplusplus
}
#endif

#endif

