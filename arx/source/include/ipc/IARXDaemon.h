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

#ifndef _IARXDAEMON_H_
#define _IARXDAEMON_H_

#include <stdint.h>
#include <binder/IInterface.h>

#include <arx/ARXStatus.h>

namespace tiarx {

class IARXClient;
class IFlatBufferMgr;
class IImageBufferMgr;

class IARXDaemon : public android::IInterface
{
public:
    DECLARE_META_INTERFACE(ARXDaemon);

    virtual arxstatus_t connect(const android::sp<IARXClient> &client) = 0;
    virtual void disconnect() = 0;

    /*!
     * Loads the specified AR Engine module into the ARXDaemon.
     */
    virtual arxstatus_t loadEngine(const char *modname) = 0;

    /*!
     * Gets the value for the specified property ID.
     * @see setProperty()
     */
    virtual arxstatus_t getProperty(uint32_t prop, int32_t *value) = 0;

    /*!
     * Sets the value for the specified property ID.
     * @see getProperty()
     */
    virtual arxstatus_t setProperty(uint32_t prop, int32_t value) = 0;

    /*!
     * Obtains the buffer queue for the given buffer ID.
     */
    virtual arxstatus_t getBufferMgr(uint32_t bufID, android::sp<IFlatBufferMgr>* mgr) = 0;
    virtual arxstatus_t getBufferMgr(uint32_t bufID, android::sp<IImageBufferMgr>* mgr) = 0;

};

class BnARXDaemon : public android::BnInterface<IARXDaemon> {
public:
    enum {
        CONNECT,
        DISCONNECT,
        LOADENGINE,
        GETPROPERTY,
        SETPROPERTY,
        GETMANAGERFLAT,
        GETMANAGERIMG,
    };

    virtual android::status_t onTransact(uint32_t code,
                                         const android::Parcel& data,
                                         android::Parcel* reply,
                                         uint32_t flags = 0);
};

}
#endif

