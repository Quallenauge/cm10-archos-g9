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

#include <ipc/IARXClient.h>
#include <binder/Parcel.h>

using namespace android;

namespace tiarx {

class BpARXClient : public android::BpInterface<IARXClient>
{
public:
    BpARXClient(const android::sp<android::IBinder>& impl)
                : android::BpInterface<IARXClient>(impl)
    {
    }

    void onPropertyChanged(uint32_t prop, int32_t value)
    {
        android::Parcel data, reply;
        data.writeInterfaceToken(IARXClient::getInterfaceDescriptor());
        data.writeInt32(prop);
        data.writeInt32(value);
        remote()->transact(BnARXClient::ONPROPERTYCHANGED, data, &reply, IBinder::FLAG_ONEWAY);
    }

};

IMPLEMENT_META_INTERFACE(ARXClient, "ti.arx.IARXClient");

// ----------------------------------------------------
status_t BnARXClient::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    status_t status = NO_ERROR;
    switch (code)
    {
        case ONPROPERTYCHANGED:
        {
            CHECK_INTERFACE(IARXClient, data, reply);
            onPropertyChanged(data.readInt32(), data.readInt32());
            break;
        }
        default:
            return (BBinder::onTransact(code, data, reply, flags));
    }
    return status;
}

}//namespace
