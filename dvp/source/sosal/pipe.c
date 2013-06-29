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

#include <sosal/pipe.h>
#include <sosal/debug.h>

#ifdef POSIX

void pipe_close(pipe_t **pp)
{
    if (pp)
    {
        pipe_t *p = *pp;
        if (p)
        {
            close(p->fd[0]);
            close(p->fd[1]);
            free(p);
            *pp = NULL;
        }
    }
}

pipe_t *pipe_open(char *name __attribute__((unused)))
{
    pipe_t *p = (pipe_t *)calloc(1, sizeof(pipe_t));
    if (p)
    {
        SOSAL_PRINT(SOSAL_ZONE_PIPE, "Created pipe %s\n", name);
        if (pipe(p->fd) < 0)
            pipe_close(&p);
    }
    return p;
}

uint32_t pipe_read(pipe_t *p, uint8_t *data, size_t len)
{
    ssize_t bytes = 0;
    bytes = read(p->fd[0], data, len);
    if (bytes < 0)
    {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to read from pipe (errno=%d)\n", errno);
        return 0;
    }
    else
    {
        return (uint32_t)bytes;
    }
}

uint32_t pipe_write(pipe_t *p, uint8_t *data, size_t len)
{
    ssize_t bytes = 0;
    bytes = write(p->fd[1], data, len);
    if (bytes < 0)
    {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to write to pipe (errno=%d)\n", errno);
        return 0;
    }
    else
    {
        return (uint32_t)bytes;
    }

}

#endif

