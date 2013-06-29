/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

#ifndef _SOSAL_PIPE_H_
#define _SOSAL_PIPE_H_

/*! \file
 * \brief The SOSAL Pipe API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>

/*! \brief A pipe construct.
 * \ingroup group_pipes
 */
typedef struct _pipe_t {
    int fd[2];              /*!< The read and write descriptors */
} pipe_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Closes a pipe
 * \param [in] pp The pointer to the pointer to the pipe_t data
 * \ingroup group_pipes
 */
void pipe_close(pipe_t **pp);

/*! \brief Opens a pipe
 * \param [in] name Opens a named pipe
 * \ingroup group_pipes
 */
pipe_t *pipe_open(char *name);

/*! \brief Reads from a pipe.
 * \param [in] p The pipe context.
 * \param [out] data The buffer to read data into.
 * \param [in] len The length of the data buffer.
 * \ingroup group_pipes
 */
uint32_t pipe_read(pipe_t *p, uint8_t *data, size_t len);

/*! \brief Writes to a pipe
 * \param [in] p The pipe context.
 * \param [in] data The buffer to write from.
 * \param [in] len The length of the data buffer.
 * \ingroup group_pipes
 */
uint32_t pipe_write(pipe_t *p, uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
