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

#include <arx/ARXBufferListener.h>

namespace tiarx {

class ARAccelerator;
class ARXFlatBufferMgr;
class DisplaySurface;
/*!
 * This class can be registered with ARX as a listener of Face detect data.
 * Upon receiving a new buffer, it renders green face boxes in an alpha surface.
 * This surface then can be used as an overlay on top of the main camera preview, which will highlight any detected faces.
 * Optionally, synchronization between rendering face boxes and the camera preview is possible
 * by using the ARX render sync methods.
 */
class FDRenderer : public ARXFlatBufferListener
{
public:
    FDRenderer(bool synchronize);
    ~FDRenderer();
    arxstatus_t init(uint32_t w, uint32_t h, uint32_t left, uint32_t top, ARAccelerator *arx, uint32_t bufID = BUFF_FACEDETECT, bool_e writeToDisk = false_e);
    void onBufferChanged(ARXFlatBuffer *pBuffer);

private:
    DisplaySurface *mSurface;
    ARAccelerator *mArx;
    mutex_t mLock;
    bool mSync;
    ARXFlatBufferMgr *mMgr;

    bool_e mWriteToDisk;
    FILE *mFile;
};

}
