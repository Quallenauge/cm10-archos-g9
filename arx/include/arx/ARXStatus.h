/*
 *  Copyright (C) 2012 Texas Instruments, Inc.
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

#ifndef _ARXSTATUS_H_
#define _ARXSTATUS_H_

#include <stdint.h>

namespace tiarx {

typedef int32_t arxstatus_t;

enum ARXStatus_e {
    NOERROR                = 0,
    FAILED                 = 0x8000000,
    INVALID_BUFFER_ID,
    INVALID_PROPERTY,
    INVALID_VALUE,
    INVALID_ARGUMENT,
    INVALID_STATE,
    NULL_POINTER,
    NOMEMORY,
    NOTCONFIGURABLE,
    UNALLOCATED_RESOURCES,
    FAILED_CREATING_ENGINE,
    FAILED_SYMBOL_LOOKUP,
    FAILED_LOADING_MODULE,
    ALREADY_HAVE_CLIENT,
    FAILED_STARTING_ENGINE,
    FAILED_STARTING_CAMERA,
};

}
#endif
