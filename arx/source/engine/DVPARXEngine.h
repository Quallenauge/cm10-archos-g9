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

#ifndef _ARXENGINE_IMPL_H_
#define _ARXENGINE_IMPL_H_

#include <engine/ARXEngine.h>

#if defined(ARX_USE_OMRONFD)
#include <FacePose.h>
#endif

namespace tiarx {

class ImageBuffer;

class DVPARXEngine : public ARXEngine
{
public:
    /** Constructor */
    DVPARXEngine();

    /** Deconstructor */
    virtual ~DVPARXEngine();

    virtual arxstatus_t SetProperty(uint32_t property, int32_t value);
    virtual arxstatus_t GetProperty(uint32_t property, int32_t *value);
    virtual arxstatus_t Setup();
    virtual arxstatus_t Process(ImageBuffer *previewBuf, ImageBuffer *videoBuf);
    virtual void Teardown();

    virtual void GraphSectionComplete(DVP_KernelGraph_t*, DVP_U32, DVP_U32);

private:
    ImageBuffer *mSobelBuf;
    ImageBuffer *mHarrisBuf;
    ImageBuffer *mGradXBuf;
    ImageBuffer *mGradYBuf;
    ImageBuffer *mGradMagBuf;
    ImageBuffer *mPreviewBuf;
    ImageBuffer *mVideoBuf;
    DVP_Buffer_t *mHarrisScratch;

    uint32_t mGradSection;
    uint32_t mSobelSection;
    uint32_t mGradNumNodes;
    int32_t mHarris_k;

#if defined(ARX_USE_OMRONFD)
    FacePose mFacePose;
#endif

};


}
#endif

