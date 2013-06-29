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

#include <sosal/profiler.h>

void profiler_clear(profiler_t *pprof)
{
    memset(pprof, 0, sizeof(profiler_t));
    pprof->rate = rtimer_freq();
    pprof->minTime = MAX_RTIMER_T;
}

void profiler_start(profiler_t *pprof)
{
    if (pprof) {
        pprof->tmpTime = rtimer_now();
    }
}

void profiler_stop(profiler_t *pprof)
{
    if (pprof)
    {
        pprof->tmpTime = (rtimer_now() - pprof->tmpTime);
        pprof->numTimes++;
        if (pprof->minTime > pprof->tmpTime)
            pprof->minTime = pprof->tmpTime;
        if (pprof->maxTime < pprof->tmpTime)
            pprof->maxTime = pprof->tmpTime;
        pprof->sumTime += pprof->tmpTime;
        pprof->avgTime = pprof->sumTime/pprof->numTimes;
    }
}

