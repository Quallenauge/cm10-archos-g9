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

#ifndef _ARXIMPL_H_
#define _ARXIMPL_H_

#include <stdint.h>

#include <arx/ARAccelerator.h>
#include <ipc/IARXClient.h>
#include <ipc/IARXDaemon.h>

#include <dvp/dvp_types.h>

namespace tiarx {

class ARXFlatBufferMgr;
class ARXImageBufferMgr;
class ARXFlatBufferMgrImpl;
class ARXImageBufferMgrImpl;

class ARXImpl :
    public ARAccelerator,
    public BnARXClient,
    public android::IBinder::DeathRecipient
{
public:
    //IARXClient methods
    void onPropertyChanged(uint32_t prop, int32_t value);

    //ARAccelerator methods
    static ARXImpl* create(ARXPropertyListener *listener, bool useDVP);
    void destroy();
    arxstatus_t loadEngine(const char *modname);

    arxstatus_t getProperty(uint32_t prop, int32_t *value);
    arxstatus_t setProperty(uint32_t prop, int32_t value);

    ARXFlatBufferMgr *getFlatBufferMgr(uint32_t bufID);
    ARXImageBufferMgr *getImageBufferMgr(uint32_t bufID);

    DVP_Handle getDVPHandle();

protected:
    //IBinder::DeathRecipient methods
    void binderDied(const android::wp<android::IBinder>& who);

    ARXImpl() {};
    ARXImpl(ARXPropertyListener *listener, bool useDVP);
    ARXImpl(const ARXImpl&);
    ARXImpl& operator=(const ARXImpl&);
    virtual ~ARXImpl();

    android::sp<IARXDaemon> mDaemon;
    ARXPropertyListener    *mListener;
    android::DefaultKeyedVector<uint32_t, android::sp<ARXFlatBufferMgrImpl> > mFlatBufMgrMap;
    android::DefaultKeyedVector<uint32_t, android::sp<ARXImageBufferMgrImpl> > mImgBufMgrMap;
    DVP_Handle mDvp;
    bool mUseDVP;
};

}
#endif //_ARXIMPL_H_
