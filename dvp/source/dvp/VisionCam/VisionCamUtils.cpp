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

#include <VisionCamUtils.h>

/** @fn DataBuffer_t::DataBuffer_t(size_t allocSize )
  * Constructor.
  * Allocate ION (other allocator) mapped memory.
  *
  * @param size_t allocSize - required size in bytes.
*/
DataBuffer_t::DataBuffer_t(size_t allocSize )
{
#ifndef DVP_USE_ION
    pData = calloc(1, allocSize);
    if ( pData)
        nBuffSize = allocSize;
    bytesWritten = 0;
#else // defined DVP_USE_ION...

    mutex_init(&mBuffLock);

    pAllocator = allocator_init();
    if( pAllocator )
    {
        dims[0].img.bpp = 1;
        dims[0].img.width = allocSize;
        dims[0].img.height = 1;
        dims[0].dim.x = allocSize;
        dims[0].dim.y = 1;
        dims[0].dim.z = 1;

        memset((void*)handles, 0, sizeof(handles));

        if( false_e == allocator_calloc( pAllocator, ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED, 1,
                                1, dims, &pData, handles, &strides )                           )
        {
            pData = NULL;
        }

        if ( pData )
            nBuffSize = allocSize;
        else
            nBuffSize = 0;

        bytesWritten = 0;
    }
#endif // DVP_USE_ION
}

/** @fn DataBuffer_t::~DataBuffer_t()
*/
DataBuffer_t::~DataBuffer_t()
{

    mutex_deinit(&mBuffLock);

#ifndef DVP_USE_ION
    FREE_IF_PRESENT( pData )
#else
    allocator_free(pAllocator, ALLOCATOR_MEMORY_TYPE_TILED_1D_UNCACHED, 1, &pData, handles);
    allocator_deinit(&pAllocator);
#endif // DVP_USE_ION
}

/** @fn void *DataBuffer_t::getData()
  * Return the user-side address of allocated buffer.
*/
void *DataBuffer_t::getData()
{
    SOSAL::AutoLock lock( &mBuffLock );
    return pData;
}

/** @fn size_t DataBuffer_t::getSize()
  * Returns the size in bytes of allocated memory.
*/
size_t DataBuffer_t::getSize()
{
    SOSAL::AutoLock lock( &mBuffLock );
    return nBuffSize;
}

/** @fn status_e DataBuffer_t::push(void* data, size_t size)
  * Copies data, pointed by 'data' with a size 'size' to the shared buffer.
  *
  * @param void *data - the data to be cpied.
  * @param size_t size - the size in bytes of this data.
*/
status_e DataBuffer_t::push(void* data, size_t size)
{
    status_e ret = STATUS_SUCCESS;

    SOSAL::AutoLock lock( &mBuffLock );

    if( (bytesWritten + size) <= nBuffSize )
    {
        memcpy((int8_t*)pData + bytesWritten, data, size);
        bytesWritten += size;
    }
    else
    {
        ret = STATUS_NO_RESOURCES;
        DVP_PRINT(DVP_ZONE_CAM, "Error: DataBuffer_t::push() - requested size is bigger than possible.\n");
        DVP_PRINT(DVP_ZONE_CAM, "Error: DataBuffer_t::push() - requested memcpy of %d bytes, buffer available are %d bytes.\n",
                  size, (nBuffSize - bytesWritten));
    }
    return ret;
}

