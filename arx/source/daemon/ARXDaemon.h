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

#ifndef _ARXDAEMON_H_
#define _ARXDAEMON_H_

#include <binder/BinderService.h>
#include <utils/KeyedVector.h>

#include <ipc/IARXDaemon.h>
#include <engine/ARXEngine.h>
#include <sosal/module.h>

namespace tiarx {

class IARXClient;
class ARXEngine;

class ARXDaemon :
    public android::BinderService<ARXDaemon>,
    public BnARXDaemon,
    public android::IBinder::DeathRecipient
{
public:
    ARXDaemon();
    virtual ~ARXDaemon();

    static char const* getServiceName() { return "ARXDaemon"; }

    //IARXDaemon
    virtual arxstatus_t connect(const android::sp<IARXClient> &client);
    virtual void disconnect();

    virtual arxstatus_t loadDefaultEngine();
    virtual arxstatus_t loadEngine(const char *modname);
    virtual arxstatus_t unloadEngine(bool unloadModule = true);

    virtual arxstatus_t getProperty(uint32_t prop, int32_t *value);
    virtual arxstatus_t setProperty(uint32_t prop, int32_t value);

    virtual arxstatus_t getBufferMgr(uint32_t bufID, android::sp<IFlatBufferMgr>* mgr);
    virtual arxstatus_t getBufferMgr(uint32_t bufID, android::sp<IImageBufferMgr>* mgr);

protected:
    virtual void binderDied(const android::wp<android::IBinder>& who);

    arxstatus_t startEngine();
    arxstatus_t stopEngine();

    int                     mState;
    android::String8        mModuleName;
    module_t                mModule;
    ARXEngineFactory_f      mFactory;
    ARXEngine               *mEngine;
    android::sp<IARXClient> mClient;

    mutex_t                 mLock;
};

} //namespace
#endif

