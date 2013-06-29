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

#ifndef _ARXBUFFERLISTENER_H_
#define _ARXBUFFERLISTENER_H_

namespace tiarx {

class ARXFlatBuffer;
class ARXImageBuffer;

/*!
 * \brief The callback interface used by ARXBufferMgr to notify clients about flat buffer changes.
 */
class ARXFlatBufferListener
{
public:
    ARXFlatBufferListener() {};
    virtual ~ARXFlatBufferListener() {};

    /*!
     * Callback received when a flat buffer changes.
     *
     */
    virtual void onBufferChanged(ARXFlatBuffer *buf) = 0;

};

/*!
 * \brief The callback interface used by ARXBufferMgr to notify clients about image buffer changes.
 */
class ARXImageBufferListener
{
public:
    ARXImageBufferListener() {};
    virtual ~ARXImageBufferListener() {};

    /*!
     * Callback received when an image buffer changes.
     *
     */
    virtual void onBufferChanged(ARXImageBuffer *buf) = 0;

};

}
#endif // _ARXBUFFERLISTENER_H_
