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

#include <sosal/queue.h>
#include <sosal/debug.h>

void queue_destroy(queue_t *q)
{
    if (q)
    {
        if (q->ringb)
            ring_destroy(q->ringb);
        event_deinit(&q->readEvent);
        event_deinit(&q->writeEvent);
        mutex_deinit(&q->access);
        free(q);
    }
}

queue_t *queue_create(size_t numMsgs, size_t msgSize)
{
    queue_t *q = (queue_t *)calloc(1, sizeof(queue_t));
    if (q != NULL)
    {
        q->msgSize = msgSize;
        q->numMsgs = numMsgs;
        q->active = true_e;
        q->popped = false_e;
        mutex_init(&q->access);
        event_init(&q->readEvent, false_e);
        event_init(&q->writeEvent, false_e);
        q->ringb = ring_create(msgSize * numMsgs);
        if (q->ringb == NULL)
        {
            queue_destroy(q);
        }
        else
        {
            // initialize the events
            event_set(&q->writeEvent);
            event_reset(&q->readEvent);
        }
    }
    return q;
}

size_t queue_length(queue_t *q)
{
    size_t length = 0;
    mutex_lock(&q->access);
    length = q->msgCount;
    mutex_unlock(&q->access);
    return length;
}

bool_e queue_write(queue_t *q, bool_e blocking, void *datum)
{
    bool_e ret = false_e;
    size_t r = 0;
    if (q && datum)
    {
        if (blocking == true_e)
        {
            SOSAL_PRINT(SOSAL_ZONE_QUEUE, "Waiting for Space in Queue %p to Write!\n", q);
            do {
                ret = event_wait(&q->writeEvent, EVENT_FOREVER);
                if (ret == false_e) {
                    SOSAL_PRINT(SOSAL_ZONE_WARNING, "WARNING! Wait for event in queue writting returned false!\n");
                } else {
                    SOSAL_PRINT(SOSAL_ZONE_QUEUE, "Queue %p has been write signaled!\n",q);
                }
            } while (ret == false_e);
        }
        mutex_lock(&q->access);
        if (q->active && !q->popped)
        {
            r = ring_write(q->ringb, datum, q->msgSize);
            if (r == q->msgSize)
            {
                q->msgCount++;
                ret = true_e;
            }
            else
            {
                ret = false_e;
            }
            if (q->ringb->numBytesFree > 0) {
                event_set(&q->writeEvent);
            } else if (q->ringb->numBytesFree == 0) {
                event_reset(&q->writeEvent);
            }
            if (q->ringb->numBytesUsed > 0) {
                event_set(&q->readEvent);
            }
        }
        else if (q->popped == true_e)
            ret = false_e;
        mutex_unlock(&q->access);
    }
    return ret;
}

bool_e queue_read(queue_t *q, bool_e blocking, void *datum)
{
    bool_e ret = false_e;
    size_t r = 0;
    if (q && datum)
    {
        if (blocking == true_e)
        {
            SOSAL_PRINT(SOSAL_ZONE_QUEUE, "Waiting for Data in Queue %p to Read!\n", q);
            do {
                ret = event_wait(&q->readEvent, EVENT_FOREVER);
                if (ret == false_e) {
                    // some one else beat us to the data
                    SOSAL_PRINT(SOSAL_ZONE_WARNING, "WARNING! Wait for the read event in the queue returned false\n");
                } else {
                    SOSAL_PRINT(SOSAL_ZONE_QUEUE, "Queue %p has been read signaled!\n",q);
                }
            } while (ret == false_e);
        }
        mutex_lock(&q->access);
        if (q->active && !q->popped)
        {
            r = ring_read(q->ringb, datum, q->msgSize);
            if (r == q->msgSize)
            {
                q->msgCount--;
                ret = true_e;
            }
            else
            {
                ret = false_e;
            }
            if (q->ringb->numBytesFree > 0) {
                event_set(&q->writeEvent);
            }
            if (q->ringb->numBytesUsed > 0) {
                event_set(&q->readEvent);
            } else if (q->ringb->numBytesUsed == 0) {
                event_reset(&q->readEvent);
            }
        }
        else if (q->popped == true_e)
            ret = false_e;
        mutex_unlock(&q->access);
    }
    return ret;
}

void queue_enable(queue_t *q)
{
    if (q)
    {
        mutex_lock(&q->access);
        if (q->active == false_e)
        {
            if (q->ringb->numBytesFree > 0) {
                event_set(&q->writeEvent);
            }
            if (q->ringb->numBytesUsed > 0) {
                event_set(&q->readEvent);
            }
            q->active = true_e;
        }
        mutex_unlock(&q->access);
    }
}

void queue_disable(queue_t *q)
{
    if (q)
    {
        mutex_lock(&q->access);
        if (q->active == true_e)
        {
            event_reset(&q->readEvent);
            event_reset(&q->writeEvent);
            q->active = false_e;
        }
        mutex_unlock(&q->access);
    }
}

void queue_pop(queue_t *q)
{
    if (q)
    {
        mutex_lock(&q->access);

        q->popped = true_e;
        event_set(&q->readEvent);
        event_set(&q->writeEvent);

        mutex_unlock(&q->access);
    }
}

queue_t *queue_reset(queue_t *q)
{
    if (q)
    {
        mutex_lock(&q->access);
        q->active = true_e;
        q->popped = false_e;

        ring_destroy(q->ringb);
        q->ringb = ring_create(q->msgSize * q->numMsgs);
        if (q->ringb == NULL)
        {
            queue_destroy(q);
            q = NULL;
        }
        else
        {
            // initialize the events
            event_set(&q->writeEvent);
            event_reset(&q->readEvent);
        }
        mutex_unlock(&q->access);
    }
    return q;
}

void queue_unpop(queue_t *q)
{
    if (q)
    {
        mutex_lock(&q->access);
        q->popped = false_e;
        event_reset(&q->readEvent);
        event_reset(&q->writeEvent);
        if (q->ringb->numBytesFree > 0) {
            event_set(&q->writeEvent);
        }
        if (q->ringb->numBytesUsed > 0) {
            event_set(&q->readEvent);
        }
        mutex_unlock(&q->access);
    }
}

bool_e queue_unittest(int argc __attribute__((unused)),
                      char *argv[] __attribute__((unused)))
{
    bool_e ret = true_e;
    uint32_t i;
    uint32_t values1[] = {1,4,8,3,2,0,4,2,99,1028,3393,203,4};
    uint32_t values2[dimof(values1)];
    queue_t *q = queue_create(dimof(values1), sizeof(uint32_t));
    if (q)
    {
        memset(values2, 0, sizeof(values2));
        for (i = 0; i < dimof(values1); i++)
            queue_write(q, true_e, &values1[i]);
        for (i = 0; i < dimof(values2); i++)
            queue_read(q, true_e, &values2[i]);

        if (memcmp(values1, values2, sizeof(values1)) != 0)
            ret = false_e;

        queue_destroy(q);
    }
    else
        ret = false_e;

    return ret;
}

