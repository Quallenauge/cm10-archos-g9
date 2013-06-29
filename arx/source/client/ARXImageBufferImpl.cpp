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

#include <ARXImageBufferImpl.h>
#include <ARXImageBufferMgrImpl.h>

#include <buffer/ImageBuffer.h>

using namespace android;

namespace tiarx {

ARXImageBufferImpl::ARXImageBufferImpl(uint32_t bufID, uint32_t index,
        const sp<ARXImageBufferMgrImpl>& mgr, const sp<ImageBuffer>& buffer)
{
    mIndex = index;
    mBuffer = buffer;
    mMgr = mgr;
    mId = bufID;
    mTimestamp = 0;
    buffer->copyInfo(&mImage);
}

ARXImageBufferImpl::~ARXImageBufferImpl()
{
    mMgr.clear();
    mBuffer.clear();
}

arxstatus_t ARXImageBufferImpl::release()
{
    decStrong(this);
    return mMgr->release(mIndex);
}

arxstatus_t ARXImageBufferImpl::render()
{
    decStrong(this);
    return mMgr->render(mIndex);
}

arxstatus_t ARXImageBufferImpl::copyInfo(DVP_Image_t *pImage)
{
    if (pImage == NULL) {
        return NULL_POINTER;
    }

    *pImage = mImage;
    return NOERROR;
}

}
