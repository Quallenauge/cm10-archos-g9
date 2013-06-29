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

#include <ARXDaemon.h>

#include <arx/ARXStatus.h>
#include <arx/ARXProperties.h>

#include <ipc/IARXClient.h>
#include <arx_debug.h>

using namespace android;

namespace tiarx {

ARXDaemon::ARXDaemon() :
        mState(ENGINE_STATE_STOP),
        mModule(NULL),
        mFactory(NULL),
        mEngine(NULL)
{
    mutex_init(&mLock);
}

ARXDaemon::~ARXDaemon()
{
    disconnect();
    mutex_deinit(&mLock);
}

void ARXDaemon::binderDied(const wp<IBinder>&)
{
    disconnect();
    unloadEngine();
}

arxstatus_t ARXDaemon::connect(const sp<IARXClient>& client)
{
    SOSAL::AutoLock lock(&mLock);
    if (mClient == NULL) {
        client->asBinder()->linkToDeath(this);
        mClient = client;
        return NOERROR;
    }

    ARX_PRINT(ARX_ZONE_WARNING, "WARNING: ARXDaemon already has a client!\n");
    return ALREADY_HAVE_CLIENT;
}

void ARXDaemon::disconnect()
{
    SOSAL::AutoLock lock(&mLock);
    if (mClient != NULL) {
        mClient->asBinder()->unlinkToDeath(this);
        mClient.clear();
    }

    //TODO: Need a way to know when all Buffer Manager references
    //have been destroyed so the module can be unloaded
    unloadEngine(false);
}

arxstatus_t ARXDaemon::loadDefaultEngine()
{
    if (mEngine == NULL) {
        //Try loading default engine
        arxstatus_t status = loadEngine("arxengine");
        if (status != NOERROR) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to load default engine!\n");
            return status;
        }
    }
    return NOERROR;
}

arxstatus_t ARXDaemon::loadEngine(const char *modname)
{
    if (mModuleName == modname && mModule != NULL) {
        ARX_PRINT(ARX_ZONE_WARNING, "Module is already loaded!\n");
        unloadEngine(false);
    } else {
        unloadEngine();

        char realpathname[MAX_PATH];
        sprintf(realpathname, MODULE_NAME("%s"), modname);
        mModule = module_load(realpathname);
        if (mModule == NULL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed loading module %s (%s)\n", realpathname, module_error());
            return FAILED_LOADING_MODULE;
        }
        mModuleName = modname;
        ARXEngineFactory_f factory = (ARXEngineFactory_f) module_symbol(mModule, "ARXEngineFactory");
        if (factory == NULL) {
            unloadEngine();
            ARX_PRINT(ARX_ZONE_ERROR, "Failed looking for ARXEngineFactory symbol\n");
            return FAILED_SYMBOL_LOOKUP;
        }
        mFactory = factory;
    }

    mEngine = mFactory();
    if (mEngine == NULL) {
        unloadEngine();
        ARX_PRINT(ARX_ZONE_ERROR, "Failed instantiating engine!\n");
        return FAILED_CREATING_ENGINE;
    }

    mEngine->SetClient(mClient);
    return NOERROR;
}

arxstatus_t ARXDaemon::unloadEngine(bool unloadModule)
{
    if (mEngine) {
        ARX_PRINT(ARX_ZONE_DAEMON, "Unloading current engine %p!\n", mEngine);
        stopEngine();
        delete mEngine;
        mEngine = NULL;
    }

    if (mModule && unloadModule) {
        ARX_PRINT(ARX_ZONE_DAEMON, "Unloading current module "FMT_MODULE_T"!\n",
                mModule);
        module_unload(mModule);
        mModule = NULL;
    }
    return NOERROR;
}

arxstatus_t ARXDaemon::startEngine()
{
    arxstatus_t status = loadDefaultEngine();
    if (status != NOERROR) {
        return status;
    }

    if (mEngine->Startup()) {
        mState = ENGINE_STATE_STOP;
        return FAILED_STARTING_ENGINE;
    }

    mState = ENGINE_STATE_START;
    return NOERROR;
}

arxstatus_t ARXDaemon::stopEngine()
{
    if (mEngine != NULL) {
        mEngine->StopThread();
    }
    mState = ENGINE_STATE_STOP;
    return NOERROR;
}

arxstatus_t ARXDaemon::setProperty(uint32_t property, int32_t value)
{
    switch (property) {
        case PROP_ENGINE_STATE:
            ARX_PRINT(ARX_ZONE_DAEMON,
                    "Current state is %d, requested state is %d\n",
                    mState, value);
            if (mState == ENGINE_STATE_STOP && value == ENGINE_STATE_START) {
                startEngine();
            } else if (mState == ENGINE_STATE_START && value == ENGINE_STATE_STOP) {
                stopEngine();
            }
            break;
        default:
            arxstatus_t status = loadDefaultEngine();
            if (status != NOERROR) {
                return status;
            }
            return mEngine->SetProperty(property, value);
    }
    return NOERROR;
}

arxstatus_t ARXDaemon::getProperty(uint32_t property, int32_t *value)
{
    switch (property) {
        case PROP_ENGINE_STATE:
            *value = mState;
            break;
        default:
            arxstatus_t status = loadDefaultEngine();
            if (status != NOERROR) {
                return status;
            }
            return mEngine->GetProperty(property, value);
    }
    return NOERROR;
}

arxstatus_t ARXDaemon::getBufferMgr(uint32_t bufID, android::sp<IFlatBufferMgr>* mgr)
{
    arxstatus_t status = loadDefaultEngine();
    if (status != NOERROR) {
        return status;
    }
    return mEngine->GetBufferMgr(bufID, mgr);
}

arxstatus_t ARXDaemon::getBufferMgr(uint32_t bufID, android::sp<IImageBufferMgr>* mgr)
{
    arxstatus_t status = loadDefaultEngine();
    if (status != NOERROR) {
        return NOERROR;
    }
    return mEngine->GetBufferMgr(bufID, mgr);
}

}
