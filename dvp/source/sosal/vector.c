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

#include <sosal/vector.h>
#include <sosal/debug.h>

float vector_mag(vector_t *v)
{
    return (float)sqrt((v->p.x*v->p.x) + (v->p.y*v->p.y) + (v->p.z*v->p.z)); // time is not considered
}

/** This vector math is of the form a += b */
void vector_add(vector_t *a, vector_t *b)
{
    a->p.x += b->p.x;
    a->p.y += b->p.y;
    a->p.z += b->p.z;
    a->p.t += b->p.t;
}

void vector_scale(vector_t *a, float b)
{
    a->p.x *= b;
    a->p.y *= b;
    a->p.z *= b;
    a->p.t *= b;
}

void vector_inverse(vector_t *a)
{
    a->p.x = -a->p.x;
    a->p.y = -a->p.y;
    a->p.z = -a->p.z;
    a->p.t = -a->p.t;
}


