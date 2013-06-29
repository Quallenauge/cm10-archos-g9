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

#if !defined(DARWIN)
#include <sosal/types.h>
#include <sosal/rtimer.h>
#else
#define MACH_TIMER
#define __LITTLE_ENDIAN__ 1
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <libkern/OSTypes.h>
//#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
typedef uint64_t rtime_t;
#define FMT_RTIMER_T "%llu"
#endif

#include <sosal/debug.h>

#ifdef SYSBIOS
#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>
#endif

#define BILLION (1000000000)

#ifdef POSIX_TIMER

rtime_t rtimer_now()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    SOSAL_PRINT(SOSAL_ZONE_TIMER, "TIME sec:%lu nsec:%lu\n",t.tv_sec,t.tv_nsec);
    return (rtime_t)((rtime_t)t.tv_nsec + ((rtime_t)t.tv_sec*BILLION));
}

rtime_t rtimer_freq()
{
    struct timespec t;
    clock_getres(CLOCK_MONOTONIC, &t);
    //SOSAL_PRINT(SOSAL_ZONE_TIMER, "FREQ sec:%lu nsec:%lu\n",t.tv_sec,t.tv_nsec);
    return (rtime_t)(BILLION/t.tv_nsec);
}

#elif defined(MACH_TIMER)

rtime_t rtimer_now()
{
    static mach_timebase_info_data_t info;
    uint64_t now_64 = mach_absolute_time();
    if (info.denom == 0)
        mach_timebase_info(&info);
    return (now_64 * info.numer)/info.denom;
}

rtime_t rtimer_freq()
{
	static mach_timebase_info_data_t info;
	if (info.denom == 0)
		mach_timebase_info(&info);
    //SOSAL_PRINT(SOSAL_ZONE_TIMER, "Numer:"FMT_RTIMER_T" Demon:"FMT_RTIMER_T"\n", info.numer, info.denom);
	return BILLION*(info.denom/info.numer);
}

#elif defined(SYSBIOS)

rtime_t rtimer_now()
{
    Types_Timestamp64               tTicks;
    uint64_t                        llTiks;


    Timestamp_get64(&tTicks);

    llTiks = ((uint64_t)(tTicks.hi) << 32) + tTicks.lo;
    return (rtime_t)(llTiks);
}

rtime_t rtimer_freq()
{
    Types_FreqHz                    tFreqHz;
    uint64_t                        llFreqHz;

    Timestamp_getFreq(&tFreqHz);

    llFreqHz = ((uint64_t)(tFreqHz.hi) << 32) + tFreqHz.lo;
    return (rtime_t)(llFreqHz);
}

#else

rtime_t rtimer_now()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return now.QuadPart;
}

rtime_t rtimer_freq()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}

#endif

rtime_t rtimer_from_rate_to_us(rtime_t t, rtime_t rate)
{
    double power = log(rate)/log(10);
    //SOSAL_PRINT(SOSAL_ZONE_TIMER, "%s: log10("FMT_RTIMER_T")=%lf\n", __FUNCTION__, rate, power);
    if (power > 6.0)
    {
        return (rtime_t)t / pow(10, power - 6.0);
    }
    else if (power < 6.0)
    {
        return (rtime_t)t * pow(10, 6.0 - power);
    }
    else
        return (rtime_t)t;
}

rtime_t rtimer_to_us(rtime_t t)
{
    return rtimer_from_rate_to_us(t, rtimer_freq());
}

double rtimer_from_rate_to_sec(rtime_t t, rtime_t rate)
{
    return (double)t/rate;
}

double rtimer_to_sec(rtime_t t)
{
    return rtimer_from_rate_to_sec(t, rtimer_freq());
}

