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

#ifndef _SOSAL_OPTIONS_H_
#define _SOSAL_OPTIONS_H_

/*! \file
 * \brief An interface to construct command line arguments.
 * \author Erik Rainey <erik.rainey@ti.com>
 * \details This is used mainly to process the argc and argv of the main function.
 * \code
 int bob = 0;
 option_t opts[] = {{OPTION_TYPE_INT. &bob, sizeof(int), "-b", "--bob", "Sets the bob variable"}};
 int main(int argc, char *argv[]) {
 	options_process(argc, argv, opts, dimof(opts));
 	printf("bob=%i\n", bob);
 }
 * \endcode
 */

#include <sosal/types.h>

/*! \brief The list of option types.
 * \ingroup group_options
 */
typedef enum _option_type_e {
    OPTION_TYPE_BOOL,	/*!< \brief A boolean type */
    OPTION_TYPE_INT,	/*!< \brief A integer type */
    OPTION_TYPE_HEX,	/*!< \brief A hexidecimal integer */
    OPTION_TYPE_FLOAT,	/*!< \brief A float number */
    OPTION_TYPE_STRING,	/*!< \brief A string type */
} option_type_e;

/*! \brief The option structure which indicates how to parse the command line arguments.
 * \ingroup group_options
 */
typedef struct _option_t {
    option_type_e type;			/*!< \brief The type of the argument. */
    void *datum;				/*!< \brief The pointer to variable of the type specified. */
    size_t size;				/*!< \brief The size of the buffer pointed to by datum */
    const char *short_switch;	/*!< \brief The string which callers can use for easy access */
    const char *long_switch;	/*!< \brief The string which callers can use for pedantic access */
    const char *description;	/*!< \brief The string printed during a "usage" call. */
} option_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This function processes the argument list based on the opts array.
 * \param [in] argc The number of arguments passed from main.
 * \param [in] argv The array of arguments passed to main.
 * \param [in] opts The option descriptions array.
 * \param [in] numOpts The number of options in the opts array.
 * \return Returns the number of arguments processed.s
 * \ingroup group_options
 */
size_t option_process(int argc, char *argv[], option_t *opts, size_t numOpts);

#ifdef __cplusplus
}
#endif

#endif

