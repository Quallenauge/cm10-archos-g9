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

#ifndef _SOSAL_DEBUG_H_
#define _SOSAL_DEBUG_H_

#include <sosal/types.h>

/*!
 * \file
 * \brief The SOSAL Debugging Zone Definitions.
 * \author Erik Rainey <erik.rainey@ti.com>
 * \details This is the manner of usage when debugging code.
\code
status = function(parameters);
if (status < 0) {
SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR: Call failed with error=%d\n", status);
}
\endcode
* or
\code
status = function(parameters);
SOSAL_ASSERT(status >= 0);
\endcode
 */

#ifdef SOSAL_DEBUG

/*! \brief A wrapper around a print.
 * \ingroup group_sosal_debug
 */
#define SOSAL_ASSERT(conditional) if (!(conditional)) {\
    printf("SOSAL_ASSERT in %s@%u: %s\n", __FILE__, __LINE__, #conditional);\
}

#if defined(WIN32) && !defined(CYGWIN)
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
/*! \brief An inline function which allows for variable arguments to the printing function
 * the MS Compiler which does not allow variable arguments in macros.
 * \param [in] conditional Typically a SOSAL Zone flag.
 * \param [in] string The format string to print.
 * \param [in] ... The variable arguments used to fill in the format string.
 * \ingroup group_sosal_debug
 */
_inline void SOSAL_PRINT(int conditional, char *string, ...) {
    if (conditional)
    {
        char format[255];
        va_list ap;
        sprintf(format, "%08x: %s", GetCurrentThreadId(), string);
        va_start(ap, string);
        vprintf(format, ap);
        va_end(ap);
    }
}
#elif defined(SYSBIOS)
/*! \brief An inline function which allows for variable arguments to the printing function
 * the TMS470 Compiler which does not allow variable arguments in macros.
 * \param [in] conditional Typically a SOSAL Zone flag.
 * \param [in] string The format string to print.
 * \param [in] ... The variable arguments used to fill in the format string.
 * \ingroup group_sosal_debug
 */
#include <stdarg.h>
#include <stdio.h> // to enable vprintf()
inline void SOSAL_PRINT(int conditional, char *string, ...) {
    if (conditional)
    {
        va_list ap;
        va_start(ap, string);
        vprintf(string, ap);
        va_end(ap);
    }
}
#else
#if (SOSAL_DEBUG == 2) && defined(ANDROID)
#include <android/log.h>
/*! \brief A wrapper around a platform specific debugging API.
 * \param [in] conditional Typically a SOSAL_ZONE_XXXX defintion.
 * \param [in] string A format string
 * \param [in] ... Variable arguments to match the format string.
 * \ingroup group_sosal_debug
 */
#define SOSAL_PRINT(conditional, string, ...) if (conditional) { __android_log_print(ANDROID_LOG_DEBUG, #conditional, string, ## __VA_ARGS__); }
#else
/*! \brief A wrapper around a platform specific debugging API.
 * \param [in] conditional Typically a SOSAL_ZONE_XXXX defintion.
 * \param [in] string A format string
 * \param [in] ... Variable arguments to match the format string.
 * \ingroup group_sosal_debug
 */
#define SOSAL_PRINT(conditional, string, ...) if (conditional) { printf(#conditional ": " string, ## __VA_ARGS__); }
#endif

#endif

#ifndef SOSAL_ZONE_MASK
#define SOSAL_RUNTIME_DEBUG
/*! \brief The runtime SOSAL debugging mask.
 * \ingroup group_sosal_mask
 */
extern uint32_t sosal_zone_mask;
#define SOSAL_ZONE_MASK       sosal_zone_mask
#endif

#else  // ifdef SOSAL_DEBUG

#define SOSAL_ASSERT(conditional, format, ...) {}
#define SOSAL_PRINT(conditional, format, ...)  {}

#endif // ifdef SOSAL_DEBUG

#define SOSAL_BIT(x)          (1 << (x))
#define SOSAL_ZONE_ERROR      (SOSAL_BIT(0) & SOSAL_ZONE_MASK) /**< Intended for error cases in all the code */
#define SOSAL_ZONE_WARNING    (SOSAL_BIT(1) & SOSAL_ZONE_MASK) /**< Intended for warning in any code */
#define SOSAL_ZONE_API        (SOSAL_BIT(2) & SOSAL_ZONE_MASK) /**< Intended for API tracing in any code */
#define SOSAL_ZONE_CACHE      (SOSAL_BIT(3) & SOSAL_ZONE_MASK)

#define SOSAL_ZONE_EVENT      (SOSAL_BIT(4) & SOSAL_ZONE_MASK)
#define SOSAL_ZONE_HASH       (SOSAL_BIT(5) & SOSAL_ZONE_MASK)
#define SOSAL_ZONE_HEAP       (SOSAL_BIT(6) & SOSAL_ZONE_MASK)
#define SOSAL_ZONE_HISTO      (SOSAL_BIT(7) & SOSAL_ZONE_MASK)

#define SOSAL_ZONE_LIST       (SOSAL_BIT(8) & SOSAL_ZONE_MASK)
#define SOSAL_ZONE_MODULE     (SOSAL_BIT(9) & SOSAL_ZONE_MASK)
#define SOSAL_ZONE_OPTIONS    (SOSAL_BIT(10)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_QUEUE      (SOSAL_BIT(11)& SOSAL_ZONE_MASK)

#define SOSAL_ZONE_SEMA       (SOSAL_BIT(12)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_SERIAL     (SOSAL_BIT(13)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_THREAD     (SOSAL_BIT(14)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_UINPUT     (SOSAL_BIT(15)& SOSAL_ZONE_MASK)

#define SOSAL_ZONE_VECTOR     (SOSAL_BIT(16)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_INI        (SOSAL_BIT(17)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_SOCKET     (SOSAL_BIT(18)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_SHARED     (SOSAL_BIT(19)& SOSAL_ZONE_MASK)

#define SOSAL_ZONE_RPC        (SOSAL_BIT(20)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_IMAGE      (SOSAL_BIT(21)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_PIPE       (SOSAL_BIT(22)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_BTREE      (SOSAL_BIT(23)& SOSAL_ZONE_MASK)

#define SOSAL_ZONE_MEM        (SOSAL_BIT(24)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_TIMER      (SOSAL_BIT(25)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_POOLMEM    (SOSAL_BIT(26)& SOSAL_ZONE_MASK)

#define SOSAL_ZONE_EXTRA      (SOSAL_BIT(31)& SOSAL_ZONE_MASK)
#define SOSAL_ZONE_ALWAYS     (1)

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief This function will query the environment to find a mask variable. It will
 * treat that mask value as a hexidecimal string. i.e. "0x10FE".
 * \param [in] mask_name The string describing the environment variable name to query.
 * \param [in] mask The poitner to the location to store the mask.
 * \ingroup group_sosal_debug
 */
void debug_get_zone_mask(char *mask_name, uint32_t *mask);

#ifdef __cplusplus
}
#endif

#endif
