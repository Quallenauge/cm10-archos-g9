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

#ifndef _ARXIMAGEBUFFERIMPL_H_
#define _ARXIMAGEBUFFERIMPL_H_

#include <utils/RefBase.h>

#include <arx/ARXStatus.h>
#include <arx/ARXImageBuffer.h>

namespace tiarx {

class ImageBuffer;
class ARXImageBufferMgrImpl;

class ARXImageBufferImpl :
    public ARXImageBuffer,
    public android::LightRefBase<ARXImageBufferImpl>
{
friend class ARXImageBufferMgrImpl;
public:

    virtual ~ARXImageBufferImpl();

    //From ARXImageBuffer
    virtual arxstatus_t release();
    virtual arxstatus_t render();
    virtual arxstatus_t copyInfo(DVP_Image_t *pImage);

private:
    /*!
     * Non-public constructor.
     */
    ARXImageBufferImpl(uint32_t bufID, uint32_t index,
                  const android::sp<ARXImageBufferMgrImpl>& mgr,
                  const android::sp<ImageBuffer>& buf);
    /*!
     * Non-public copy constructor.
     */
    ARXImageBufferImpl(const ARXImageBufferImpl&);
    /*!
     * Non-public assignment operator.
     */
    ARXImageBufferImpl& operator=(const ARXImageBufferImpl&);

    inline void setTimestamp(uint64_t timestamp) { mTimestamp = timestamp; }

    uint32_t mIndex;
    android::sp<ImageBuffer> mBuffer;
    android::sp<ARXImageBufferMgrImpl> mMgr;
};

}
#endif // _ARXIMAGEBUFFERIMPL_H_
