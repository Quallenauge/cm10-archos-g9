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


#ifndef _BNFLATBUFFERMGR_H_
#define _BNFLATBUFFERMGR_H_

#include <buffer/IFlatBufferMgr.h>
#include <buffer/IBufferMgrTemplate.h>

namespace tiarx {

class BnFlatBufferMgr : public BnBufferMgr<IFlatBufferMgr>
{
public:
    enum {
        GETBUFFER = BnBufferMgr<IFlatBufferMgr>::FIRST_DERIVED_TRANSACTION,
        SETSIZE,
    };

    virtual android::status_t onTransact(uint32_t code,
                                           const android::Parcel& data,
                                           android::Parcel* reply,
                                           uint32_t flags = 0);
};

}

#endif //_BNFLATBUFFERMGR_H_
