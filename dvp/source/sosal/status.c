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

#include <sosal/types.h>
#include <sosal/status.h>

#if defined(WIN32) && !defined(CYGWIN)
 /*! \brief Used to co-relate the Windows codes to status_e.
  * \ingroup group_status
  */
 status_to_error correlations[] = {
     {STATUS_NO_MAPPING,         ERROR_NOT_ENOUGH_MEMORY},
     {STATUS_NOT_ENOUGH_MEMORY,  ERROR_NOT_ENOUGH_MEMORY},
     {STATUS_INVALID_PARAMETER,  ERROR_INVALID_PARAMETER},
     {STATUS_INVALID_STATE,      ERROR_BAD_PIPE},
     {STATUS_NOT_IMPLEMENTED,    ERROR_CALL_NOT_IMPLEMENTED},
     {STATUS_NO_RESOURCES,       ERROR_BUSY},
     {STATUS_TIMEDOUT,           ERROR_TIMEOUT},
     {STATUS_CATASTROPHIC,       ERROR_GEN_FAILURE},
     {STATUS_FAILURE,            ERROR_GEN_FAILURE},
     {STATUS_END_OF_FILE,        ERROR_HANDLE_EOF},
     {STATUS_FILE_NOT_FOUND,     ERROR_FILE_NOT_FOUND},
     {STATUS_SUCCESS,            1},
 };
#elif defined(SYSBIOS)
/*! \brief Used to co-relate the SYSBIOS codes to status_e.
 * \ingroup group_status
 */
status_to_error correlations[] = {
    {STATUS_SUCCESS,            0},
};
#else
/*! \brief Used to co-relate the POSIX codes to status_e.
 * \ingroup group_status
 */
status_to_error correlations[] = {
     {STATUS_NO_MAPPING,         EADDRNOTAVAIL},
     {STATUS_NOT_ENOUGH_MEMORY,  ENOMEM},
     {STATUS_INVALID_PARAMETER,  EINVAL},
     {STATUS_INVALID_STATE,      ENOSYS},
     {STATUS_NOT_IMPLEMENTED,    ENOTSUP},
     {STATUS_NO_RESOURCES,       0},
     {STATUS_TIMEDOUT,           ETIMEDOUT},
     {STATUS_CATASTROPHIC,       ENOSYS},
     {STATUS_FAILURE,            0},
     {STATUS_END_OF_FILE,        0},
     {STATUS_FILE_NOT_FOUND,     0},
     {STATUS_SUCCESS,            1},
 };
#endif

long status_convert(status_e status)
{
    uint32_t i = 0;
    for (i = 0; i < dimof(correlations); i++)
    {
        if (status == correlations[i].status)
            return correlations[i].code;
    }
    return -1; // generic error just about everywhere.
}

