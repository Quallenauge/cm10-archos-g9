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

#ifndef _SOSAL_PROFILER_H_
#define _SOSAL_PROFILER_H_

/*! \file
 * \brief The SOSAL Profiling API
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>
#include <sosal/rtimer.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief The profiling data structure.
 * \ingroup group_profilers
 */
typedef struct _profiler_t {
    uint32_t numTimes;  /**< \brief Internally used to help calculate the timing info. */
    rtime_t tmpTime;    /**< \brief Used to record the last start time. After \ref profiler_stop, this contains the last session time.  */
    rtime_t minTime;    /**< \brief Used to record the minimum time of execution */
    rtime_t maxTime;    /**< \brief Used to record the maximum time of execution */
    rtime_t avgTime;    /**< \brief Used to record the average time of execution */
    rtime_t sumTime;    /**< \brief Used to record the total time of execution of all iterations */
    rtime_t rate;       /**< \brief This is used to record the clock rate per second on the local core */
} profiler_t;

/*! \brief Clears the profiling data structure and initializes it to known good values.
 * \param [in,out] pprof The pointer to the profiling data structure.
 * \ingroup group_profilers
 */
void profiler_clear(profiler_t *pprof);

/*! \brief Starts a profiling session.
 * \param [in,out] pprof The pointer to the profiling data structure.
 * \ingroup group_profilers
 */
void profiler_start(profiler_t *pprof);

/*! \brief Stops a profiling session and updates the profiling data structure.
 * \param [in,out] pprof The pointer to the profiling data structure.
 * \ingroup group_profilers
 */
void profiler_stop(profiler_t *pprof);

#ifdef __cplusplus
}
#endif

#endif

