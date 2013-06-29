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

#include <sosal/bitfield.h>
#include <sosal/debug.h>

#define BIT_TST(field, bit) (((field) >> bit) & 1)
#define BIT_CLR(field, bit) ((field) &= ~(1<<bit))
#define BIT_SET(field, bit) ((field) |= (1<<bit))

static uint32_t bitfield_length(bitfield_t *b)
{
    uint32_t len = (b->max/32);
    len += (b->max % 32 == 0?0:1);
    return len * 4;
}

void bitfield_deinit(bitfield_t *b)
{
    if (b)
    {
        if (b->fields)
        {
            memset(b->fields, 0, bitfield_length(b));
            free(b->fields);
            b->fields = NULL;
        }
        b->max = 0;
        b->count = 0;
    }
}

void bitfield_init(bitfield_t *b, uint32_t max)
{
    if (b)
    {
        b->count = 0;
        b->max = max;
        if (b->fields)
        {
            memset(b->fields, 0, bitfield_length(b));
        }
        else
        {
            b->fields = (uint32_t *)calloc(1, bitfield_length(b));
        }
        //printf("Allocated a bitfield of %u bits (%u byte length)\n", max, bitfield_length(b));
    }
}

bool_e bitfield_rls(bitfield_t *b, uint32_t bit)
{
    if (b && b->fields)
    {
        uint32_t c = 0; // calculated bit
        uint32_t d = 0; // intrafield bit
        uint32_t i = 0; // index
        uint32_t l = bitfield_length(b) / 4;
        for (i = 0; i < l; i++)
        {
            for (d = 0; d < 32; d++)
            {
                c = d + (i * 32);
                if (c == bit && BIT_TST(b->fields[i], d))
                {
                    BIT_CLR(b->fields[i], d);
                    return true_e;
                }
            }
        }
    }
    return false_e;
}

bool_e bitfield_get(bitfield_t *b, uint32_t *bit)
{
    *bit = 0xFFFFFFFF;
    if (b && b->fields)
    {
        uint32_t c = 0; // calculated bit
        uint32_t d = 0; // intrafield bit
        uint32_t i = 0; // index
        uint32_t l = bitfield_length(b) / 4;
        for (i = 0; i < l; i++)
        {
            for (d = 0; d < 32; d++)
            {
                // the calculated bit
                c = d + (i * 32);

                // if the bit is higher than is allowed, fail
                if (c >= b->max)
                    return false_e;

                if (!BIT_TST(b->fields[i], d))
                {
                    BIT_SET(b->fields[i], d);
                    *bit = c;
                    return true_e;
                }
            }
        }
    }
    return false_e;
}

uint32_t bitfield_count(bitfield_t *b)
{
    if (b)
        return b->max;
    else
        return 0;
}


