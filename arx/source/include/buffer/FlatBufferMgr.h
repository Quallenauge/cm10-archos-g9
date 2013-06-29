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

#ifndef _FLATBUFFERMGR_H_
#define _FLATBUFFERMGR_H_

//#include <buffer/IFlatBufferMgr.h>
#include <buffer/BnFlatBufferMgr.h>
#include <buffer/BufferMgr.h>

namespace tiarx {

class FlatBufferMgr :
    public BnFlatBufferMgr,
    public BufferMgr
{
public:
    FlatBufferMgr(uint32_t bufId, uint32_t size,
                  DVP_MemType_e memType = DVP_MTYPE_DEFAULT,
                  uint32_t count = DEFAULT_COUNT,
                  bool lockConfig = true,
                  bool holdReady = false);
    virtual ~FlatBufferMgr();

    inline uint32_t size() const { return mSize; }

    //From BufferMgr
    arxstatus_t free();

    FlatBuffer *nextFree();
    FlatBuffer *nextReady();

    //From IFlatBufferMgr
    virtual arxstatus_t getBuffer(uint32_t index, android::sp<FlatBuffer> *buffer);
    virtual arxstatus_t setSize(uint32_t size);

    //From IBufferMgr
    virtual arxstatus_t release(uint32_t index);

private:
    arxstatus_t allocateBuffer(uint32_t index);
    uint32_t mSize;
    sp<FlatBuffer> mBuffers[IBufferMgr::MAX_BUFFERS];
};

}

#endif //_FLATBUFFERMGR_H_
