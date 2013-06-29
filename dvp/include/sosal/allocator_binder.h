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

#ifndef _SOSAL_BINDER_HELPER_H_
#define _SOSAL_BINDER_HELPER_H_

/*! \file
 * \brief The Android C++ wrappers for sharing SOSAL allocated memory over a Binder.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/allocator.h>

#if defined(ANDROID) && defined(__cplusplus)

#if defined(FROYO) || defined(GINGERBREAD) || defined(ICS) || defined(JELLYBEAN)
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

using android::Parcel;

/*! \brief The C++ Namespace for SOSAL components. */
namespace SOSAL {

/*! \brief Writes a \ref allocator_memory_t meta-data structure to a parcel.
 * \ingroup group_allocators
 */
void WriteMemoryToParcel(Parcel &parcel, allocator_memory_t *memory);

/*! \brief Reads a \ref allocator_memory_t meta-data structure from a parcel.
 * \ingroup group_allocators
 */
void ReadMemoryFromParcel(const Parcel &parcel, allocator_memory_t *memory);
};

#else // NDK

#include <linux/binder.h>

#endif // IF MYDROID

#endif // IF ANDROID && C++

#endif

