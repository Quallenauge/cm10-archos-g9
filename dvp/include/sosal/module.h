/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

#ifndef _SOSAL_MODULE_H_
#define _SOSAL_MODULE_H_

/*! \file
 * \brief The SOSAL Module API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>

#define SOSAL_MODULE_API

#if defined(ANDROID) || defined(LINUX) || defined(CYGWIN) || defined(DARWIN) || defined(__QNX__)

#define POSIX_DL
#undef WIN32_DL
#undef SYSBIOS_SL

#include <dlfcn.h>
#if defined(CYGWIN)
#ifndef RTLD_LOCAL
#define RTLD_LOCAL (0)
#endif
#endif

#elif defined(WIN32)

#undef POSIX_DL
#define WIN32_DL
#undef SYSBIOS_SL

#elif defined(SYSBIOS)

#undef POSIX_DL
#undef WIN32_DL
#define SYSBIOS_SL

#else  // Unknown version

#undef POSIX_DL
#undef WIN32_DL
#undef SYSBIOS_SL

#endif // OSes

#if defined(POSIX_DL)

/*! \brief The module handle.
 * \ingroup group_modules
 */
typedef void *module_t;

/*! \brief The symbol handle.
 * \ingroup group_modules
 */
typedef void *symbol_t;
#if defined(DARWIN)
#define MODULE_NAME(name)    "lib"name".dylib"
#elif defined(CYGWIN)
#define MODULE_NAME(name)   "lib"name".dll.a"
#else // QNX, LINUX
#define MODULE_NAME(name)   "lib"name".so"
#endif
#define PATH_DELIM          "/"
#define FMT_MODULE_T        "%p"
#if defined(__QNX__)
#define MODULE_PATH         PATH_DELIM"usr"PATH_DELIM"lib"PATH_DELIM
#else
#define MODULE_PATH         ""
#endif
#define MODULE_EXPORT

#elif defined(WIN32_DL)

/*! \brief The module handle.
 * \ingroup group_modules
 */
typedef HMODULE module_t;

/*! \brief The symbol handle.
 * \ingroup group_modules
 */
typedef FARPROC symbol_t;
#define MODULE_NAME(name)   name".dll"
#define PATH_DELIM          "\\"
#define MODULE_PATH         "C:"PATH_DELIM"Windows"PATH_DELIM"system32"PATH_DELIM
#define FMT_MODULE_T        "%p"
#define MODULE_EXPORT        __declspec(dllexport)

#elif defined(SYSBIOS_SL)

#define MODULE_NAME(name)    name
#define PATH_DELIM           "_"
#define MODULE_PATH          ""
#define FMT_MODULE_T         "%p"
#define MODULE_EXPORT        static

/*! \brief A generic function pointer.
 * \ingroup group_modules
 */
typedef void (*function_f)(void);

/*! \brief A function is a symbol too.
 * \ingroup group_modules
 */
typedef function_f symbol_t;

/*! \brief Ties a function with a symbol name.
 * \ingroup group_modules
 */
typedef struct _static_function_t {
    char name[MAX_PATH];
    function_f func_ptr;
} static_function_t;

/*! \brief Describes the static linking table of a module.
 * \ingroup group_modules
 */
typedef struct _static_module_table_t {
    char name[MAX_PATH];
    uint32_t numFunctions;
    static_function_t *functions;
} static_module_table_t;

/*! \brief A module in SYSBIOS is a static module.
 * \ingroup group_modules
 */
typedef static_module_table_t *module_t;

#else // No implementation

#undef SOSAL_MODULE_API

#endif // implementations

#if defined(SOSAL_MODULE_API)

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This function loads a module from the file system or from a static table.
 * \param [in] filename The name of the module to load.
 * \return Returns the handle to the module.
 * \retval 0 if the module was not found.
 * \post \ref module_unload
 * \ingroup group_modules
 * \note Clients should use \ref MODULE_NAME to generically load modules.
 */
module_t module_load(const char *filename);

/*! \brief This function tries to get the handle to a symbol from a
 * module.
 * \param [in] mod The handle to the module.
 * \return Returns the symbol handle which can be cast into a function pointer.
 * \retval NULL if no such symbol exists.
 * \pre \ref module_load
 * \post \ref module_unload
 * \ingroup group_modules
 * \note Typically only "C" functions are loaded. However this will
 * load any symbol name, which means if you know the managling
 * mechanism, other symbols are accessible.
 */
symbol_t module_symbol(module_t mod, const char *symbol_name);

/*! \brief This returns a pointer to an error string.
 * \return Returns the error string.
 * \ingroup group_modules
 */
char *module_error();

/*! \brief This function unloads a module.
 * \param [in] mod The module to unload.
 * \pre \ref module_load
 * \ingroup group_modules
 * \note The HLOS will typically keep an internal reference count, keeping
 * the module in memory after the unload is called.
 */
void module_unload(module_t mod);

#ifdef __cplusplus
}
#endif

#endif // SOSAL_MODULE_API

#endif // _SOSAL_MODULE_H_

