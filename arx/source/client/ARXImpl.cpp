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

#include <binder/IServiceManager.h>

#include <ARXImpl.h>
#include <ARXFlatBufferMgrImpl.h>
#include <ARXImageBufferMgrImpl.h>
#include <arx/ARXProperties.h>

#include <dvp/dvp_api.h>

using namespace android;

namespace tiarx {

ARXImpl::ARXImpl(ARXPropertyListener *listener, bool useDVP)
    : mListener(listener)
{
    if (useDVP) {
        mDvp = DVP_KernelGraph_Init();
    } else {
        mDvp = DVP_MemImporter_Create();
    }
    mUseDVP = useDVP;
}

ARXImpl::~ARXImpl()
{
    if (mDvp) {
        if (mUseDVP) {
            DVP_KernelGraph_Deinit(mDvp);
        } else {
            DVP_MemImporter_Free(mDvp);
        }
    }
}

void ARXImpl::destroy()
{
    for (size_t i = 0; i < mFlatBufMgrMap.size(); i++) {
        const sp<ARXFlatBufferMgrImpl>& mgr = mFlatBufMgrMap.valueAt(i);
        mgr->freeBuffers();
    }

    for (size_t i = 0; i < mImgBufMgrMap.size(); i++) {
        const sp<ARXImageBufferMgrImpl>& mgr = mImgBufMgrMap.valueAt(i);
        mgr->freeBuffers();
    }

    mFlatBufMgrMap.clear();
    mImgBufMgrMap.clear();
    if (mDaemon != NULL) {
        mDaemon->asBinder()->unlinkToDeath(this);
        mDaemon->disconnect();
        mDaemon.clear();
    }
    decStrong(this);
}

void ARXImpl::binderDied(const wp<IBinder>&)
{
    mDaemon.clear();
    mListener->onPropertyChanged(PROP_ENGINE_STATE, ENGINE_STATE_DEAD);
}

arxstatus_t ARXImpl::loadEngine(const char *modname)
{
    mFlatBufMgrMap.clear();
    mImgBufMgrMap.clear();
    return mDaemon->loadEngine(modname);
}

arxstatus_t ARXImpl::setProperty(uint32_t property, int32_t value)
{
    return mDaemon->setProperty(property, value);
}

arxstatus_t ARXImpl::getProperty(uint32_t property, int32_t *value)
{
    return mDaemon->getProperty(property, value);
}

void ARXImpl::onPropertyChanged(uint32_t property, int32_t value)
{
    //ARX_PRINT(ARX_ZONE_CLIENT, "Property %u changed to %u\n", property, value);
    mListener->onPropertyChanged(property, value);
}

ARXImpl *ARXImpl::create(ARXPropertyListener *listener, bool useDVP)
{
    sp<IARXDaemon> daemon;
    getService<IARXDaemon>(android::String16("ARXDaemon"), &daemon);

    if (daemon == NULL) {
        return NULL;
    }

    sp<ARXImpl> arx = new ARXImpl(listener, useDVP);
    arxstatus_t status = daemon->connect(arx);
    if (status == NOERROR) {
        daemon->asBinder()->linkToDeath(arx);
        arx->mDaemon = daemon;
    } else {
        return NULL;
    }

    arx->incStrong(arx.get());
    return arx.get();
}

ARXImageBufferMgr *ARXImpl::getImageBufferMgr(uint32_t bufID)
{
    sp<ARXImageBufferMgrImpl> mgrProxy = mImgBufMgrMap.valueFor(bufID);
    if (mgrProxy == NULL) {
       sp<IImageBufferMgr> mgr;
       arxstatus_t status = mDaemon->getBufferMgr(bufID, &mgr);
       if (mgr != NULL && status == NOERROR) {
           mgrProxy = new ARXImageBufferMgrImpl(bufID, mgr, mDvp);
           mImgBufMgrMap.add(bufID, mgrProxy);
       }
    }
    return mgrProxy.get();
}

ARXFlatBufferMgr *ARXImpl::getFlatBufferMgr(uint32_t bufID)
{
    sp<ARXFlatBufferMgrImpl> mgrProxy = mFlatBufMgrMap.valueFor(bufID);
    if (mgrProxy == NULL) {
       sp<IFlatBufferMgr> mgr;
       arxstatus_t status = mDaemon->getBufferMgr(bufID, &mgr);
       if (mgr != NULL && status == NOERROR) {
           mgrProxy = new ARXFlatBufferMgrImpl(bufID, mgr, mDvp);
           mFlatBufMgrMap.add(bufID, mgrProxy);
       }
    }
    return mgrProxy.get();
}

DVP_Handle ARXImpl::getDVPHandle()
{
    if (mUseDVP) {
        return mDvp;
    }
    //Don't return just the mem importer handle, as it's not useful to the client.
    return 0;
}

ARAccelerator *ARAccelerator::create(ARXPropertyListener *listener)
{
    return ARXImpl::create(listener, false);
}

ARAccelerator *ARAccelerator::create(ARXPropertyListener *listener, bool useDVP)
{
    return ARXImpl::create(listener, useDVP);
}

}// namespace
