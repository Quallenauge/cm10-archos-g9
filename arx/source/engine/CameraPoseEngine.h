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

#ifndef _TICAMERAPOSEENGINE_H_
#define _TICAMERAPOSEENGINE_H_

#include <engine/ARXEngine.h>

#include <arti/LIBARTI.h>
#include <arti/ARLIB.h>

namespace tiarx {

class ImageBuffer;

struct ARTIInfo {
    float cameraCalParams[9];
    float projectionMatrix[12];
    float cameraCenter[4];
};

class CameraPoseEngine : public ARXEngine
{
public:
    /** Constructor */
    CameraPoseEngine();

    /** Deconstructor */
    virtual ~CameraPoseEngine();

    virtual arxstatus_t SetProperty(uint32_t property, int32_t value);
    virtual arxstatus_t GetProperty(uint32_t property, int32_t *value);
    virtual arxstatus_t Setup();
    virtual arxstatus_t Process(ImageBuffer *previewBuf, ImageBuffer *videoBuf);
    virtual void Teardown();

    /** Overloaded to set AR specific requirements */
    arxstatus_t DelayedCamera2ALock();

private:
    AR_handle mARHandle;
    AR_image mARImage;
    ARTIInfo mARInfo;

    bool mUseCPUAlgo;
    bool mResetTracking;
    bool mEnableGausFilter;
    uint32_t mInitialDelay;
    DVP_Perf_t mPerf;

    DVP_Image_t mImages[6];
    DVP_Buffer_t mBuffers[4];

};


}
#endif

