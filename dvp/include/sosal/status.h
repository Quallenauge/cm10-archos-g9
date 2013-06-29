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

#ifndef _SOSAL_STATUS_H_
#define _SOSAL_STATUS_H_

/*! \file
 * \brief
 * \author Erik Rainey <erik.rainey@ti.com>
 */

/*! \brief The error code type for SOSAL.
 * \ingroup group_status
 */
typedef enum _sosal_error_e {
     STATUS_BASE              = -600,
     STATUS_NO_MAPPING        = STATUS_BASE+1, /**< Failed to map memory */
     STATUS_NOT_ENOUGH_MEMORY = STATUS_BASE+2, /**< Returned when there is not sufficient memory to run */
     STATUS_INVALID_PARAMETER = STATUS_BASE+3, /**< One of the supplied parameters is invalid */
     STATUS_INVALID_STATE     = STATUS_BASE+4, /**< The component is in the wrong state to take this action */
     STATUS_NOT_IMPLEMENTED   = STATUS_BASE+5, /**< Requested parameter is not implemented */
     STATUS_NO_RESOURCES      = STATUS_BASE+6, /**< Not enough resource to complete action, not explicity memory related */
     STATUS_TIMEDOUT          = STATUS_BASE+7, /**< A timed event has expired */
     STATUS_CATASTROPHIC      = STATUS_BASE+8, /**< Usually a hardware error */
     STATUS_FAILURE           = STATUS_BASE+9, /**< Generic failure code */
     STATUS_END_OF_FILE       = STATUS_BASE+10,/**< End of file reached while reading */
     STATUS_FILE_NOT_FOUND    = STATUS_BASE+11,/**< The system cannot find the file specified */
     STATUS_SUCCESS           = 1,             /**< Success! */
 } status_e;

 /*! \brief A Co-relation structure for long cores to status_e.
  * \ingroup group_status
  */
 typedef struct _status_corelation_e {
     status_e status;   /*!< The \ref status_e code */
     long     code;     /*!< The system code */
 } status_to_error;


 /*! \brief Used to check for passing codes */
#define STATUS_PASSED(x)    (x == STATUS_SUCCESS)
 /*! \brief Used to check for failing codes */
#define STATUS_FAILED(x)    (x < STATUS_SUCCESS)

/*! \brief This function converts a status_e code to a platform specific code.
 * \ingroup group_status
 */
long status_convert(status_e status);

#endif

