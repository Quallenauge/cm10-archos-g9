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

#ifndef _SOSAL_TYPES_H_
#define _SOSAL_TYPES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h> // this defines the standard types
#include <math.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>

#if defined(WIN32) && !defined(CYGWIN)
#define snprintf _snprintf
#endif

#if defined(ANDROID) || defined(LINUX) || defined(DARWIN) || defined(__QNX__) || defined(CYGWIN)
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/mman.h>
#define POSIX
#elif defined(WIN32)
#undef POSIX
#elif defined(SYSBIOS)
#undef POSIX
/*! SYSBIOS lacks a ssize_t definition */
typedef signed long ssize_t;
#ifndef ARCH_32
// for SYSBIOS - currently support 32 bit architectures...
#define ARCH_32
#endif
#else
#error Unsupported HLOS!
#endif

// DEFINES for using different types in PRINTF
#if defined(WIN32) && !defined(CYGWIN)
#define FMT_SIZE_T    "%lu"
#define FMT_SSIZE_T   "%ld"
#elif defined(SYSBIOS)
#define FMT_SIZE_T    "%lu"
#define FMT_SSIZE_T   "%ld"
#elif defined(ANDROID)
#define FMT_SIZE_T    "%zu"
#define FMT_SSIZE_T   "%ld"
#else
#define FMT_SIZE_T    "%zu"
#define FMT_SSIZE_T   "%zd"
#endif

#if defined(ANDROID) || defined(SYSBIOS)
// these platforms only currently support 32 bit architectures...
#ifndef ARCH_32
#define ARCH_32
#endif
#endif

/*! A C boolean work-a-like
 * \see true_e
 */
enum _bool_e {
    false_e = 0,    /*!< FALSE */
};

/*! The boolean type for C */
typedef enum _bool_e bool_e;

/*! True is defined as NOT false_e
 * \see bool_e
 */
#define true_e (bool_e)(!false_e)

/*! The basic pointer type on all systems. This grows with the bit depth of the architecture. */
typedef void *ptr_t;

/*! The basic interchangable value holder on any system. This grows with the bit depth of the architecture. */
typedef size_t value_t;

/*! \brief To be used inline with a printing function for the formating string
 * of a value_t.
\code
printf("Value="FMT_VALUE_T"\n", value);
\endcode
 */
#ifdef SYSBIOS
/*
 * SYSBIOS supports next conversion specifiers
 * d, i, u, x, o, p, c, s, f  and l as length modifier
 */
#define FMT_VALUE_T   "%lu"
#else
#define FMT_VALUE_T   "%zu"
#endif

#if defined(ARCH_32) || defined(DARWIN)
#define FMT_CONST     "%u"
#define FMT_INT64_T   "%lld"
#define FMT_UINT64_T  "%llu"
#define FMT_HEX64_T   "%llx"
/** Used to specify in a format string a unsigned 64 bit value of a certain number of digits */
#define FMT_U64(d) "%"#d"llu"
#elif defined(ARCH_64) 
#define FMT_CONST     "%lu"
#define FMT_INT64_T   "%ld"
#define FMT_UINT64_T  "%lu"
#define FMT_HEX64_T   "%lx"
/** Used to specify in a format string a unsigned 64 bit value of a certain number of digits */
#define FMT_U64(d) "%"#d"lu"
#else
#error message("You must define your ARCH_XX size, either 32 or 64 bits.")
#endif

#ifndef dimof
/*! Returns the number of elements in an array of items */
#define dimof(x)    (sizeof(x)/sizeof(x[0]))
#endif

#ifndef STRINGERIZE
/*! A wrapper around the stringerizer macro */
#define STRINGERIZE(x)  #x
#endif

#ifndef MAX_PATH
/*! \brief Maximum file system path value (common across many HLOS, but not maximal on each) */
#define MAX_PATH        (255)
#endif

#ifndef SOSAL_PAGE_SIZE
#if defined(LINUX) && defined(PAGE_SIZE)
/*! \brief Used to reference the system page size */
#define SOSAL_PAGE_SIZE PAGE_SIZE
#else
/*! \brief Used to define the system page size if not defined */
#define SOSAL_PAGE_SIZE (4096)
#endif
#endif

#if defined(WIN32)
/*! \brief Used to define the compiler trick to cause structure packing */
#define PACKED_STRUCT(x)
#else
/*! \brief Used to define the compiler trick to cause structure packing */
#define PACKED_STRUCT(x)   __attribute__((aligned(x)))
#endif

#endif
