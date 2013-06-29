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

#include <sosal/ring.h>
#include <sosal/debug.h>
#ifdef SYSOSAL
#include <xdc/runtime/Assert.h> //for assert support
#endif

ring_t *ring_create(size_t totalNumBytes)
{
    ring_t *rb = (ring_t *)calloc(1,sizeof(ring_t));
    if (rb == NULL)
         return rb;

    rb->start = (uint8_t *)calloc(totalNumBytes, sizeof(uint8_t));
    if (rb->start == NULL)
    {
        free(rb);
        return NULL;
    }

    rb->totalNumBytes = totalNumBytes;
    rb->front = rb->start;
    rb->back = rb->start;
    rb->end = &(rb->start[rb->totalNumBytes]);  // this must the point PAST the
                                                // end of the buffer!
    rb->numBytesFree = totalNumBytes;
    rb->numBytesUsed = 0;
    return rb;
}

void ring_destroy(ring_t *rb)
{
    if (rb == NULL)
        return;

    // destroy the raw buffer
    if (rb->start != NULL)
        free(rb->start);

    // destroy the structure
    if (rb != NULL)
        free(rb);
}

size_t ring_write(ring_t *rb, uint8_t *b, size_t len)
{
    size_t numBytesWritten = 0;

    // if I couldn't fit the data, don't write it.
    if (len > rb->numBytesFree) {
        return 0;
    }
    if (len == 1)
    {
        // since we have at least one byte free, we don't have to worry about
        // the pointer problems associated with 1+ buffer copies
        *rb->back++ = *b;
        if (rb->back == rb->end)
            rb->back = rb->start;
        rb->numBytesUsed++;
        rb->numBytesFree--;
        return 1;
    }

    // non-inverted (back is higher than front)
    if (rb->back > rb->front)
    {
        size_t end_to_back = (rb->end - rb->back);

        if (end_to_back > len)
        {
            memcpy(rb->back,b,len);
            rb->back += len;
            rb->numBytesFree -= len;
            rb->numBytesUsed += len;
            numBytesWritten = len;
        }
        else
        {
            // this wraps the ring buffer around
            size_t left_over = len - end_to_back;

            memcpy(rb->back,b,end_to_back);
            rb->back = rb->start;
            rb->numBytesFree -= end_to_back;
            rb->numBytesUsed += end_to_back;

            if (left_over > 0)
            {
                memcpy(rb->back,&(b[end_to_back]),left_over);
                rb->back += left_over;
                rb->numBytesFree -= left_over;
                rb->numBytesUsed += left_over;
            }
            numBytesWritten = len;
        }
    }
    // inverted pointers (front is higher than back) ring has wrapped
    else if (rb->front > rb->back)
    {
        size_t front_to_back = (rb->front - rb->back);
        if(len <= front_to_back )
        {
            memcpy(rb->back,b,len);
            rb->back += len;
            rb->numBytesFree -= len;
            rb->numBytesUsed += len;

            numBytesWritten = len;
        }
        else
        {
            assert(len <= front_to_back);
        }
    }
    else // they're equal
    {
        if (rb->numBytesFree == 0)
        {
            // buffer is full, we shouldn't have arrived here
            assert(rb->numBytesFree != 0);
            // impossible code path...
        }
        else if (rb->numBytesUsed == 0)
        {
            // completely empty

            if (len > rb->totalNumBytes)
            {
                // buffer can't fit in ring, should not have arrived here.
                assert(len <= rb->totalNumBytes);
                // impossible code path...
            }
            else if (len <= rb->totalNumBytes)
            {
                size_t end_to_back = (size_t)(rb->end - rb->back);

                if (len < end_to_back)
                {
                    memcpy(rb->back,b,len);
                    rb->back += len;
                    rb->numBytesFree -= len;
                    rb->numBytesUsed += len;
                    numBytesWritten = len;
                }
                else
                {
                    // this wraps the ring buffer around
                    size_t left_over = len - end_to_back;

                    memcpy(rb->back,b,end_to_back);
                    rb->back = rb->start;
                    rb->numBytesFree -= end_to_back;
                    rb->numBytesUsed += end_to_back;

                    if (left_over > 0)
                    {
                        memcpy(rb->back,&(b[end_to_back]),left_over);
                        rb->back += left_over;
                        rb->numBytesFree -= left_over;
                        rb->numBytesUsed += left_over;
                    }

                    numBytesWritten = len;
                }
            }
        }
        else
        {
            // error
            assert((rb->numBytesFree + rb->numBytesUsed) == rb->totalNumBytes);
            // TODO if there is an error in the private data, reinitialize...
        }
    }

    // LeaveCriticalSection(&rb->cs);

    return numBytesWritten;
}

size_t ring_read(ring_t *rb, uint8_t *b, size_t len)
{
    size_t numBytesRead = 0;

    if (rb->numBytesUsed == 0) {
        // LeaveCriticalSection(&rb->cs);
        return numBytesRead;
    }
    if (len == 1)
    {
        // since there is at least one byte in the ring, we don't have to worry
        // about the complexities of the byte processing
        *b = *rb->front++;
        if (rb->front == rb->end)
            rb->front = rb->start;
        rb->numBytesUsed--;
        rb->numBytesFree++;
        return 1;
    }

    // non-inverted
    if (rb->back > rb->front)
    {
        size_t back_to_front = (rb->back - rb->front);

        if(len < back_to_front)
            numBytesRead = len;
        else
            numBytesRead = back_to_front;

        memcpy(b,rb->front,numBytesRead);
        rb->front += numBytesRead;
        rb->numBytesFree += numBytesRead;
        rb->numBytesUsed -= numBytesRead;
    }
    // inverted case (ring wrapped)
    else if (rb->front > rb->back)
    {
        size_t end_to_front = (rb->end - rb->front);
        if (len < end_to_front)
        {
            memcpy(b,rb->front,len);
            rb->front += len;
            rb->numBytesFree += len;
            rb->numBytesUsed -= len;
            numBytesRead = len;
        }
        else
        {
            size_t left_over = len - end_to_front;
            size_t readLen = 0;

            memcpy(b,rb->front,end_to_front);
            rb->front = rb->start;
            rb->numBytesFree += end_to_front;
            rb->numBytesUsed -= end_to_front;

            if (left_over > 0)
            {
                size_t back_to_start = (rb->back - rb->start);
                if (left_over <= back_to_start)
                    readLen = left_over;
                else
                    readLen = back_to_start;
                if (readLen > 0)
                {
                    memcpy(&(b[end_to_front]),rb->front,readLen);
                    rb->front += readLen;
                    rb->numBytesFree += readLen;
                    rb->numBytesUsed -= readLen;
                }
            }
            numBytesRead = end_to_front + readLen;
        }
    }
    else // they're equal
    {
        if (rb->numBytesFree == 0)
        {
            // full buffer
            size_t end_to_front = (rb->end - rb->front);
            size_t readLen = 0;
            if (len < end_to_front)
            {
                memcpy(b,rb->front,len);
                rb->front += len;
                rb->numBytesFree += len;
                rb->numBytesUsed -= len;
                numBytesRead = len;
            }
            else
            {
                size_t left_over = len - end_to_front;
                memcpy(b,rb->front,end_to_front);
                rb->front = rb->start;
                rb->numBytesFree += end_to_front;
                rb->numBytesUsed -= end_to_front;
                if (left_over > 0)
                {
                    size_t back_to_start = (rb->back - rb->start);
                    if (left_over <= back_to_start)
                        readLen = left_over;
                    else
                        readLen = back_to_start;

                    if (readLen > 0)
                    {
                        memcpy(&(b[end_to_front]),rb->front,readLen);
                        rb->front += readLen;
                        rb->numBytesFree += readLen;
                        rb->numBytesUsed -= readLen;
                    }
                }
                numBytesRead = end_to_front + readLen;
            }
        }
        else if (rb->numBytesUsed == 0)
        {
            // empty buffer, we should not have gotten here
            assert(rb->numBytesUsed != 0);
            // impossible code path...
        }
        else
        {
            // error
            assert((rb->numBytesFree + rb->numBytesUsed) == rb->totalNumBytes);

#ifdef RINGB_PROACTIVE
            // there are bytes left in the array,
            // but we don't know how we skipped them.
            // we have to assume that start, end and totalNumBytes are ok.
            rb->numBytesFree = rb->totalNumBytes;
            rb->numBytesUsed = 0;
            rb->front = rb->start;
            rb->back = rb->front;
            numBytesRead = 0;
#endif
        }
    }
    return numBytesRead;
}

void ring_print(ring_t *rb)
{
    size_t i;

    printf("Ring Buffer:\n");
    printf("================================\n");
    printf("Total Size: "FMT_SIZE_T" byte(s)\n",rb->totalNumBytes);
    printf(" Free Size: "FMT_SIZE_T" byte(s)\n",rb->numBytesFree);
    printf(" Used Size: "FMT_SIZE_T" byte(s)\n",rb->numBytesUsed);
    printf("Start Pointer: %p\n",rb->start);
    printf("  End Pointer: %p\n",rb->end);
    printf("Front Pointer: %p ["FMT_SIZE_T"]\n",rb->front,(rb->front-rb->start));
    printf(" Back Pointer: %p ["FMT_SIZE_T"]\n",rb->back,(rb->back-rb->start));

    if (rb->back > rb->front)
    {
        uint8_t *tmp = rb->start;
        for (i = 0; i < rb->totalNumBytes; i++)
        {
            if (tmp < rb->front)
                printf("-- ");
            else if (tmp < rb->back)
                printf("%02x ",*tmp);
            else
                printf("-- ");
            tmp++;
        }
        printf("\n");

        assert(rb->numBytesFree > 0);
        assert(rb->numBytesUsed > 0);
    }
    else if (rb->front > rb->back)
    {
        uint8_t *tmp = rb->start;
        for (i = 0; i < rb->totalNumBytes; i++)
        {
            if (tmp < rb->back)
                printf("%02x ",*tmp);
            else if (tmp < rb->front)
                printf("-- ");
            else
                printf("%02x ",*tmp);
            tmp++;
        }
        printf("\n");

        assert(rb->numBytesFree > 0);
        assert(rb->numBytesUsed > 0);
    }
    else
    {
        if (rb->numBytesFree == 0)
        {
            uint8_t *tmp = rb->start;
            for (i = 0; i < rb->totalNumBytes; i++)
            {
                printf("%02x ",*tmp);
                tmp++;
            }
            printf("\n");

            assert(rb->numBytesUsed == rb->totalNumBytes);
        }
        else if (rb->numBytesUsed == 0)
        {
            uint8_t *tmp = rb->start;
            for (i = 0; i < rb->totalNumBytes; i++)
            {
                printf("-- ");
                tmp++;
            }
            printf("\n");

            assert(rb->numBytesFree == rb->totalNumBytes);
        }
    }
}

