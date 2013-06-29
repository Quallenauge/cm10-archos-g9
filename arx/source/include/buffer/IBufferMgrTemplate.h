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

#ifndef _IBUFFERMGRTEMPLATE_H_
#define _IBUFFERMGRTEMPLATE_H_

#include <binder/IInterface.h>
#include <buffer/IBufferMgrClient.h>
#include <arx_debug.h>

namespace tiarx {

template<typename INTERFACE>
class BnBufferMgr : public android::BnInterface<INTERFACE>
{
public:
    enum {
        BINDGRAPH = android::IBinder::FIRST_CALL_TRANSACTION,
        SETCOUNT,
        GETCOUNT,
        RELEASE,
        REGISTERCLIENT,
        FIRST_DERIVED_TRANSACTION
    };
    virtual android::status_t onTransact(uint32_t code,
                                           const android::Parcel& data,
                                           android::Parcel* reply,
                                           uint32_t flags = 0)
    {
        android::status_t status = NO_ERROR;

        switch (code) {
            case BINDGRAPH:
            {
                CHECK_INTERFACE(INTERFACE, data, reply);
                // @TODO de-serialize here
                arxstatus_t ret = this->bindGraph(NULL);
                reply->writeInt32(ret);
                break;
            }
            case RELEASE:
            {
                CHECK_INTERFACE(INTERFACE, data, reply);
                uint32_t index = data.readInt32();
                arxstatus_t ret = this->release(index);
                reply->writeInt32(ret);
                break;
            }
            case REGISTERCLIENT:
            {
                CHECK_INTERFACE(INTERFACE, data, reply);
                android::sp<IBufferMgrClient> client = interface_cast<IBufferMgrClient>(data.readStrongBinder());
                arxstatus_t ret = this->registerClient(client.get());
                reply->writeInt32(ret);
                break;
            }
            case SETCOUNT:
            {
                CHECK_INTERFACE(INTERFACE, data, reply);
                uint32_t count = data.readInt32();
                arxstatus_t ret = this->setCount(count);
                reply->writeInt32(ret);
                break;
            }
            case GETCOUNT:
            {
                CHECK_INTERFACE(INTERFACE, data, reply);
                reply->writeInt32(this->getCount());
                break;
            }
            default :
                return android::BBinder::onTransact(code, data, reply, flags);
        }
        return status;
    }
};

template<typename INTERFACE>
class BpBufferMgr : public android::BpInterface<INTERFACE>
{
public:

    BpBufferMgr(const android::sp<android::IBinder>& impl) :
        android::BpInterface<INTERFACE>(impl)
    {
    }

    arxstatus_t bindGraph(DVP_KernelGraph_t *)
    {
        android::Parcel data, reply;
        data.writeInterfaceToken(this->getInterfaceDescriptor());
        //@TODO serialize graph
        this->remote()->transact(BnBufferMgr<INTERFACE>::BINDGRAPH, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t release(uint32_t index)
    {
        android::Parcel data, reply;
        data.writeInterfaceToken(this->getInterfaceDescriptor());
        data.writeInt32(index);
        this->remote()->transact(BnBufferMgr<INTERFACE>::RELEASE, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t registerClient(IBufferMgrClient *client)
    {
        android::Parcel data, reply;
        data.writeInterfaceToken(this->getInterfaceDescriptor());
        data.writeStrongBinder(client->asBinder());
        this->remote()->transact(BnBufferMgr<INTERFACE>::REGISTERCLIENT, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t setCount(uint32_t count)
    {
        android::Parcel data, reply;
        data.writeInterfaceToken(this->getInterfaceDescriptor());
        data.writeInt32(count);
        this->remote()->transact(BnBufferMgr<INTERFACE>::SETCOUNT, data, &reply);
        return reply.readInt32();
    }

    uint32_t getCount() const
    {
       android::Parcel data, reply;
       data.writeInterfaceToken(this->getInterfaceDescriptor());
       this->remote()->transact(BnBufferMgr<INTERFACE>::GETCOUNT, data, &reply);
       return reply.readInt32();
    }
};

} // namespace

#endif //_IBUFFERMGRTEMPLATE_H_
