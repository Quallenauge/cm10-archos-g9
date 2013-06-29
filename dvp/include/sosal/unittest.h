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

#ifndef _SOSAL_UNITTEST_H_
#define _SOSAL_UNITTEST_H_

/*! 
 * \file 
 * \brief The unit test interface for SOSAL. 
 * \author Erik Rainey <erik.rainey@ti.com>
 */
 
#include <sosal/types.h>

/*! \brief A function pointer to a unit test.
 * \param [in] argc The number of arguments.
 * \param [in] argv The array of arguments. 
 * \ingroup group_unittest
 */
typedef bool_e (*unittest_f)(int argc, char *argv[]);

/*! \brief A descriptor of a unit test. 
 * \ingroup group_unittest
 */
typedef struct _unittest_t {
    char name[MAX_PATH];	/*!< Name of the unit test */
    unittest_f test;		/*!< Function pointer to the unit test */
    bool_e noArgs;			/*!< Indicates whether the unit test requires arguments. */
} unittest_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e allocator_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e cache_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e event_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e thread_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e queue_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e hash_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e option_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e heap_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e list_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e histogram_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e ini_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e fourcc_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e threadpool_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e shared_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e socket_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e rpc_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e module_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e uinput_unittest(int argc, char *argv[]);

/*! \brief 
 * \param [in] argc
 * \param [in] argv
 * \ingroup group_unittest
 */
bool_e btreelist_unittest(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif
