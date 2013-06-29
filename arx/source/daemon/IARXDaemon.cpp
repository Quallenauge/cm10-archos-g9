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

#include <ipc/IARXDaemon.h>
#include <ipc/IARXClient.h>
#include <buffer/IFlatBufferMgr.h>
#include <buffer/IImageBufferMgr.h>
#include <arx_debug.h>

#include <binder/Parcel.h>

using namespace android;

namespace tiarx {

class BpARXDaemon : public BpInterface<IARXDaemon>
{
public:
    BpARXDaemon(const sp<IBinder>& impl)
        : BpInterface<IARXDaemon>(impl)
    {
    }

    arxstatus_t connect(const sp<IARXClient> &client)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IARXDaemon::getInterfaceDescriptor());
        data.writeStrongBinder(client->asBinder());
        remote()->transact(BnARXDaemon::CONNECT, data, &reply);
        return reply.readInt32();
    }

    void disconnect()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IARXDaemon::getInterfaceDescriptor());
        remote()->transact(BnARXDaemon::DISCONNECT, data, &reply);
    }

    arxstatus_t loadEngine(const char *modname)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IARXDaemon::getInterfaceDescriptor());
        data.writeCString(modname);
        remote()->transact(BnARXDaemon::LOADENGINE, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t getProperty(uint32_t prop, int32_t *value)
    {
        Parcel    data, reply;
        if (value == NULL) {
            return NULL_POINTER;
        }
        data.writeInterfaceToken(IARXDaemon::getInterfaceDescriptor());
        data.writeInt32(prop);
        remote()->transact(BnARXDaemon::GETPROPERTY, data, &reply);
        *value = reply.readInt32();
        return reply.readInt32();
    }

    arxstatus_t setProperty(uint32_t prop, int32_t value)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IARXDaemon::getInterfaceDescriptor());
        data.writeInt32(prop);
        data.writeInt32(value);
        remote()->transact(BnARXDaemon::SETPROPERTY, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t getBufferMgr(uint32_t bufID, android::sp<IFlatBufferMgr>* mgr)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IARXDaemon::getInterfaceDescriptor());
        data.writeInt32(bufID);
        remote()->transact(BnARXDaemon::GETMANAGERFLAT, data, &reply);
        arxstatus_t status = reply.readInt32();
        if (status == NOERROR) {
            *mgr = interface_cast<IFlatBufferMgr>(reply.readStrongBinder());
        }
        return status;
    }

    arxstatus_t getBufferMgr(uint32_t bufID, android::sp<IImageBufferMgr>* mgr)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IARXDaemon::getInterfaceDescriptor());
        data.writeInt32(bufID);
        remote()->transact(BnARXDaemon::GETMANAGERIMG, data, &reply);
        arxstatus_t status = reply.readInt32();
        if (status == NOERROR) {
            *mgr = interface_cast<IImageBufferMgr>(reply.readStrongBinder());
        }
        return status;
    }
};

IMPLEMENT_META_INTERFACE(ARXDaemon, "ti.arx.IARXDaemon");

// ----------------------------------------------------------------------

status_t BnARXDaemon::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    status_t    status = NO_ERROR;

    switch( code ) {
        case CONNECT:
        {
            CHECK_INTERFACE(IARXDaemon, data, reply);
            sp<IARXClient> client = interface_cast<IARXClient>(data.readStrongBinder());
            status = reply->writeInt32(connect(client));
            break;
        }
        case DISCONNECT:
        {
            CHECK_INTERFACE(IARXDaemon, data, reply);
            disconnect();
            break;
        }
        case LOADENGINE:
        {
            CHECK_INTERFACE(IARXDaemon, data, reply);
            status = reply->writeInt32(loadEngine(data.readCString()));
            break;
        }
        case GETPROPERTY:
        {
            CHECK_INTERFACE(IARXDaemon, data, reply);
            uint32_t prop = data.readInt32();
            int32_t value;
            arxstatus_t ret = getProperty(prop, &value);
            reply->writeInt32(value);
            reply->writeInt32(ret);
            break;
        }
        case SETPROPERTY:
        {
            CHECK_INTERFACE(IARXDaemon, data, reply);
            uint32_t prop = data.readInt32();
            int32_t value = data.readInt32();
            arxstatus_t ret = setProperty(prop, value);
            status = reply->writeInt32(ret);
            break;
        }
        case GETMANAGERFLAT:
        {
            CHECK_INTERFACE(IARXDaemon, data, reply);
            uint32_t bufID = data.readInt32();
            sp<IFlatBufferMgr> mgr;
            arxstatus_t ret = getBufferMgr(bufID, &mgr);
            status = reply->writeInt32(ret);
            if (mgr != NULL && status == NO_ERROR) {
                status = reply->writeStrongBinder(mgr->asBinder());
            }
            break;
        }
        case GETMANAGERIMG:
        {
            CHECK_INTERFACE(IARXDaemon, data, reply);
            uint32_t bufID = data.readInt32();
            sp<IImageBufferMgr> mgr;
            arxstatus_t ret = getBufferMgr(bufID, &mgr);
            status = reply->writeInt32(ret);
            if (mgr != NULL && status == NO_ERROR) {
                status = reply->writeStrongBinder(mgr->asBinder());
            }
            break;
        }
        default :
            return (BBinder::onTransact(code, data, reply, flags));
    }
    return (status);
}


} // namespace
