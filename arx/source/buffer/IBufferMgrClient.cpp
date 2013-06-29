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

#include <buffer/IBufferMgrClient.h>
#include <binder/Parcel.h>

using namespace android;

namespace tiarx {

class BpBufferMgrClient : public android::BpInterface<IBufferMgrClient>
{
public:
    BpBufferMgrClient(const android::sp<android::IBinder>& impl)
                : android::BpInterface<IBufferMgrClient>(impl)
    {
    }

    void onBufferChanged(uint32_t index, uint64_t timestamp)
    {
        android::Parcel data, reply;
        data.writeInterfaceToken(IBufferMgrClient::getInterfaceDescriptor());
        data.writeInt32(index);
        data.writeInt64(timestamp);
        remote()->transact(BnBufferMgrClient::ONBUFFERCHANGED, data, &reply, IBinder::FLAG_ONEWAY);
    }

};

IMPLEMENT_META_INTERFACE(BufferMgrClient, "ti.arx.IBufferMgrClient");

// ----------------------------------------------------
status_t BnBufferMgrClient::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    status_t status = NO_ERROR;
    switch (code)
    {
        case ONBUFFERCHANGED:
        {
            CHECK_INTERFACE(IBufferMgrClient, data, reply);
            onBufferChanged(data.readInt32(), data.readInt64());
            break;
        }
        default:
            return (BBinder::onTransact(code, data, reply, flags));
    }
    return status;
}

}//namespace
