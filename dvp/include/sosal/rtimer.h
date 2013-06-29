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

#ifndef _SOSAL_RTIMER_H_
#define _SOSAL_RTIMER_H_

/*! \file
 * \brief The SOSAL High Resolution Timer API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>

#if defined(ANDROID) || defined(LINUX) || defined(__QNX__) || defined(CYGWIN) || defined(DARWIN)
#define POSIX_TIMER
#undef WIN32_TIMER
#undef BIOS_TIMER
#elif defined(WIN32)
#undef POSIX_TIMER
#define WIN32_TIMER
#undef BIOS_TIMER
#elif defined(SYSBIOS)
#undef POSIX_TIMER
#undef WIN32_TIMER
#define BIOS_TIMER
#endif

#if defined(POSIX_TIMER)
/*! On POSIX environments, 64 bit is needed. 
 * \ingroup group_rtimers
 */
typedef uint64_t rtime_t;
#define MAX_RTIMER_T    ((rtime_t)0xFFFFFFFFFFFFFFFFULL)
#define FMT_RTIMER_T    FMT_UINT64_T
#endif

#if defined(WIN32_TIMER)
/*! On WIN32 environments, 64 bit is needed. 
 * \ingroup group_rtimers
 */
typedef LONGLONG rtime_t;
#define MAX_RTIMER_T    0xFFFFFFFFFFFFFFFF
#define FMT_RTIMER_T    "%I64d"
#endif

#if defined(BIOS_TIMER)
/*! On SYSBIOS environments, 64 bit is needed. 
 * \ingroup group_rtimers
 */
typedef unsigned long long rtime_t;
#define MAX_RTIMER_T    ((rtime_t)0xFFFFFFFFFFFFFFFFULL)
#define FMT_RTIMER_T    FMT_UINT64_T 
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Gets the current time.
 * \return Returns the current time.
 * \ingroup group_rtimers
 */
rtime_t rtimer_now();

/*! \brief Gets the frequency of the timer.
 * \return returns the frequency of the timer.
 * \ingroup group_rtimers
 */
rtime_t rtimer_freq();

/*! \brief Converts the rtime_t to microseconds.
 * \param [in] t The rtime_t to convert.
 * \return Returns an rtime_t scaled into microseconds.
 * \ingroup group_rtimers
 */
rtime_t rtimer_to_us(rtime_t t);

/*! \brief Converts the rtimer_t from an externally supplied rate 
 * into microseconds.
 * \param [in] t The time to convert.
 * \param [in] rate The external rate.
 * \return Returns an rtime_t which is converted.
 * \ingroup group_rtimers
 */
rtime_t rtimer_from_rate_to_us(rtime_t t, rtime_t rate);

/*! \brief Converts the rtime_t to seconds.
 * \param [in] t The rtime_t to convert.
 * \return Returns an rtime_t converted into seconds in a double.
 * \ingroup group_rtimers
 */
double rtimer_to_sec(rtime_t);

/*! \brief Converts the rtimer_t from an externally supplied rate 
 * into seconds.
 * \param [in] t The time to convert.
 * \param [in] rate The external rate.
 * \return Returns an rtime_t which is converted into a double in seconds.
 * \ingroup group_rtimers
 */
double rtimer_from_rate_to_sec(rtime_t, rtime_t rate);

#ifdef __cplusplus
}
#endif

#endif

