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
#include <buffer/IFlatBufferMgr.h>
#include <buffer/BnFlatBufferMgr.h>

using namespace android;

namespace tiarx {

class BpFlatBufferMgr : public BpBufferMgr<IFlatBufferMgr>
{
public:
    BpFlatBufferMgr(const sp<IBinder>& impl) :
        BpBufferMgr<IFlatBufferMgr>(impl)
    {
    }

    arxstatus_t getBuffer(uint32_t index, sp<FlatBuffer> *buffer)
    {
        Parcel data, reply;
        data.writeInterfaceToken(getInterfaceDescriptor());
        data.writeInt32(index);
        remote()->transact(BnFlatBufferMgr::GETBUFFER, data, &reply);
        arxstatus_t status = reply.readInt32();
        if (status == NOERROR) {
            *buffer = FlatBuffer::readFromParcel(reply);
        }
        return status;
    }

    arxstatus_t setSize(uint32_t size)
    {
        Parcel data, reply;
        data.writeInterfaceToken(getInterfaceDescriptor());
        data.writeInt32(size);
        remote()->transact(BnFlatBufferMgr::SETSIZE, data, &reply);
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(FlatBufferMgr, "ti.arx.IFlatBufferMgr");

// ----------------------------------------------------------------------

status_t BnFlatBufferMgr::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    status_t status = NO_ERROR;

    switch (code) {
        case GETBUFFER:
        {
            CHECK_INTERFACE(IFlatBufferMgr, data, reply);
            uint32_t index = data.readInt32();
            sp<FlatBuffer> buf;
            arxstatus_t ret = getBuffer(index, &buf);
            reply->writeInt32(ret);
            if (ret == NOERROR) {
                FlatBuffer::writeToParcel(buf, reply);
            }
            break;
        }
        case SETSIZE:
        {
            CHECK_INTERFACE(IFlatBufferMgr, data, reply);
            uint32_t size = data.readInt32();
            arxstatus_t ret = setSize(size);
            reply->writeInt32(ret);
            break;
        }
        default :
            return BnBufferMgr<IFlatBufferMgr>::onTransact(code, data, reply, flags);
    }
    return status;
}

} // namespace
