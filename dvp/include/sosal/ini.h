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

#ifndef _SOSAL_INI_H_
#define _SOSAL_INI_H_

/*!
 * \file
 * \brief A simple INI parser. 
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>
#include <sosal/debug.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief INI parser callback used when the \ref ini_parse encounters a triple. 
 * \param [in] arg The user private data.
 * \param [in] section The string describing the section. 
 * \param [in] variable The string describing the variable. 
 * \param [in] value The string describing the value.
 * \ingroup group_ini
 */
typedef void (*ini_callback_f)(void *arg, char *section, char *variable, char* value);

/*! \brief A list of capitalizations in INI files.
 * \ingroup group_ini
 */
enum _ini_chars_e { 
    INI_NATURAL,    /*!< Returns characters as they appear in the INI file. */
    INI_LOWER,      /*!< Returns characters as lower case only */
    INI_UPPER,      /*!< Returns characters as upper case only */
};

/*! \brief Formal type definition for character capitalization types.
 * \ingroup group_ini
 */
typedef enum _ini_chars_e ini_chars_e;

/*! 
 * \brief This function will take the input buffer and parse out INI formatted variable/value pairs. 
 * \param [in] buffer The input characters, usually read from a file.
 * \param [in] len the length of the buffer. 
 * \param [in] callback The callback function to give the parsed sction, variable and value set to.
 * \param [in] char_case The enumerated value of which capitalization scheme is used.
 * \param [in] arg A pointer that will be passed to the callback function. This can be used to hold a private structure or cookie.
 * \ingroup group_ini
 */
int ini_parse(char *buffer, size_t len, ini_callback_f callback, ini_chars_e char_case, void *arg);

#ifdef __cplusplus
}
#endif

#endif


