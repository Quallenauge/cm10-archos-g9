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

#include <sosal/sosal.h>

namespace SOSAL {

void WriteMemoryToParcel(Parcel &parcel, allocator_memory_t *memory)
{
    int32_t p, n;
    parcel.writeInt32(memory->memtype);
    parcel.writeInt32(memory->nptrs);
    parcel.writeInt32(memory->ndims);
    for (p = 0; p < memory->nptrs; p++) {
        for(n = 0; n < memory->ndims; n++) {
            parcel.writeInt32(memory->dims[p].dims[n]);
        }
    }
    for (p = 0; p < memory->nptrs; p++) {
        for(n = 0; n < memory->ndims; n++) {
            parcel.writeInt32(memory->strides[p].dims[n]);
        }
    }
    for (p = 0; p < memory->nptrs; p++) {
        if (memory->memtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED ||
            memory->memtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED)
            parcel.writeFileDescriptor(memory->fds[p]);
        else if (memory->memtype == ALLOCATOR_MEMORY_TYPE_VIRTUAL_SHARED)
            parcel.writeInt32(memory->fds[p]);
    }
}

void ReadMemoryFromParcel(const Parcel &parcel, allocator_memory_t *memory)
{
    int32_t p, n;
    memory->memtype = (allocator_memory_type_e)parcel.readInt32();
    memory->nptrs = parcel.readInt32();
    memory->ndims = parcel.readInt32();
    for (p = 0; p < memory->nptrs; p++) {
        for(n = 0; n < memory->ndims; n++) {
            memory->dims[p].dims[n] = parcel.readInt32();
        }
    }
    for (p = 0; p < memory->nptrs; p++) {
        for(n = 0; n < memory->ndims; n++) {
            memory->strides[p].dims[n] = parcel.readInt32();
        }
    }
    for (p = 0; p < memory->nptrs; p++) {
        if (memory->memtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_CACHED ||
            memory->memtype == ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED)
            memory->fds[p] = parcel.readFileDescriptor();
        else if (memory->memtype == ALLOCATOR_MEMORY_TYPE_VIRTUAL_SHARED)
            memory->fds[p] = parcel.readInt32();
    }
}

};

