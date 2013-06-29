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

#include <buffer/FlatBuffer.h>
#include <buffer/IBufferMgrClient.h>
#include <dvp/dvp_mem.h>
#include <dvp/dvp_api.h>

#include <arx_debug.h>

using namespace android;

namespace tiarx {

FlatBuffer::FlatBuffer(uint32_t index, uint32_t size, DVP_Handle dvp, DVP_MemType_e type,
                       sp<IBufferMgrClient>& client, queue_t *freeQ, queue_t *readyQ) :
        Buffer(index, dvp, client, freeQ, readyQ, false)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mSize = size;
    DVP_Buffer_Init(&mBuffer, 1, size);
    mBuffer.memType = type;
    mSharedFd = 0;
    mImportHdl = 0;
}

FlatBuffer::FlatBuffer() :
        Buffer(0, 0, NULL, NULL, NULL, false)
{
    DVP_Buffer_Init(&mBuffer, 1, 0);
    mSharedFd = 0;
    mImportHdl = 0;
    mSize = 0;
}

FlatBuffer::~FlatBuffer()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    free();
}

arxstatus_t FlatBuffer::copy(DVP_Buffer_t *image)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    *image = mBuffer;
    return NOERROR;
}

arxstatus_t FlatBuffer::allocate()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (mRemote) {
        ARX_PRINT(ARX_ZONE_API, "%s: trying to allocate remote buffer!\n", __FUNCTION__);
        return INVALID_STATE;
    }
    bool ret = DVP_Buffer_Alloc(mDvp, &mBuffer, static_cast<DVP_MemType_e>(mBuffer.memType));
    return ret ? NOERROR : FAILED;
}

arxstatus_t FlatBuffer::free()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (mRemote) {
        if (mImportHdl != 0) {
            DVP_Buffer_Free_Import(mDvp, &mBuffer, mImportHdl);
            mImportHdl = 0;
            if (mBuffer.memType != DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
                close(mSharedFd);
            }
        } else {
            ARX_PRINT(ARX_ZONE_ERROR, "FlatBuffer::%s, No valid import handle!\n", __FUNCTION__);
        }
    } else {
        DVP_Buffer_Free(mDvp, &mBuffer);
        if (mBuffer.memType != DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
            close(mSharedFd);
        }
    }
    return NOERROR;
}

arxstatus_t FlatBuffer::import(DVP_Handle dvp)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (!mRemote) {
        ARX_PRINT(ARX_ZONE_API, "%s: Trying to import a non-remote buffer!\n", __FUNCTION__);
        return INVALID_STATE;
    }
    mDvp = dvp;
    ARX_PRINT(ARX_ZONE_API, "%s: Trying to import buffer with shared_fd:%d...\n", __FUNCTION__, mSharedFd);
    bool ret = DVP_Buffer_Import(mDvp, &mBuffer, mSharedFd, &mImportHdl);
    if (ret) {
        DVP_PrintBuffer(ARX_ZONE_API, &mBuffer);
    }
    return ret ? NOERROR : FAILED;
}

arxstatus_t FlatBuffer::writeToParcel(const sp<FlatBuffer>& buffer, Parcel* parcel)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    parcel->writeInt32(buffer->mIndex);
    parcel->writeInt32(buffer->mBuffer.elemSize);
    parcel->writeInt32(buffer->mBuffer.numBytes);
    parcel->writeInt32(buffer->mBuffer.memType);

    bool ret = DVP_Buffer_Share(buffer->mDvp, &buffer->mBuffer, &buffer->mSharedFd);
    if (ret) {
        if (buffer->mBuffer.memType == DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
            parcel->writeInt32(buffer->mSharedFd);
        } else {
            parcel->writeDupFileDescriptor(buffer->mSharedFd);
        }
        return NOERROR;
    }
    return FAILED;
}

sp<FlatBuffer> FlatBuffer::readFromParcel(const Parcel& data)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    sp<FlatBuffer> buffer = new FlatBuffer();
    buffer->mRemote = true;
    buffer->mIndex = data.readInt32();
    DVP_Buffer_t *pBuffer = &buffer->mBuffer;
    pBuffer->elemSize = data.readInt32();
    pBuffer->numBytes = data.readInt32();
    pBuffer->memType = data.readInt32();
    if (pBuffer->memType == DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
       buffer->mSharedFd = data.readInt32();
    } else {
       buffer->mSharedFd = dup(data.readFileDescriptor());
    }
    ARX_PRINT(ARX_ZONE_API, "%s: shared_fd:%d\n", __FUNCTION__, buffer->mSharedFd);
    return buffer;
}

bool FlatBuffer::internalRelease()
{
    return true;
}

}
