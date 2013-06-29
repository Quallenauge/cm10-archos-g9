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

#ifndef _ARXENGINE_H_
#define _ARXENGINE_H_

#include <arx/ARXStatus.h>
#include <ipc/IARXClient.h>
#include <dvp/VisionEngine.h>

namespace tiarx {

class IFlatBufferMgr;
class IImageBufferMgr;
class FlatBufferMgr;
class ImageBufferMgr;
class ImageBuffer;
class FlatBuffer;
class IARXClient;

class ARXEngine : public VisionEngine
{
public:
    /** Constructor */
    ARXEngine();

    /** Deconstructor */
    virtual ~ARXEngine();

    virtual arxstatus_t SetProperty(uint32_t property, int32_t value);
    virtual arxstatus_t GetProperty(uint32_t property, int32_t *value);
    virtual arxstatus_t GetBufferMgr(uint32_t bufID, android::sp<IFlatBufferMgr> *mgr);
    virtual arxstatus_t GetBufferMgr(uint32_t bufID, android::sp<IImageBufferMgr> *mgr);
    void SetClient(const android::sp<IARXClient>& client) { mClient = client; }

    virtual arxstatus_t Setup();
    virtual arxstatus_t Process(ImageBuffer *previewBuf, ImageBuffer *videoBuf);
    virtual void Teardown();
    virtual arxstatus_t DelayedCamera2ALock();

    //Overrides of VisionEngine
    virtual status_e Engine();
    virtual void ReceiveImage(VisionCamFrame * cameraFrame);
    //virtual VisionCamFrame *DequeueImage();
    void Shutdown();
    /** Overloaded to set AR specific requirements */
    status_e DelayedCameraFocusSetting();

    //We won't use these but we have to implement them
    virtual status_e GraphSetup() { return STATUS_FAILURE; }
    virtual status_e GraphUpdate(VisionCamFrame*) { return STATUS_FAILURE; }
    virtual void GraphSectionComplete(DVP_KernelGraph_t*, DVP_U32, DVP_U32) {}

protected:
    virtual arxstatus_t copyFaceDetect(FlatBuffer *buf, VisionCamFaceType *faces, uint32_t numFaces);
    android::DefaultKeyedVector<uint32_t, android::sp<ImageBufferMgr> > mImgBuffMgrMap;
    android::DefaultKeyedVector<uint32_t, android::sp<FlatBufferMgr> > mFlatBuffMgrMap;
    android::sp<IARXClient> mClient;

    VisionCamMirrorType mMainMirror;
    VisionCamMirrorType mSecMirror;

    uint32_t mCam2ALockDelay;
};


/** Used to cast the symbol_t from the module into the function call */
typedef ARXEngine *(*ARXEngineFactory_f)(void);

/** This is the module interface used to load the engine object once the module is in memory */
extern "C" ARXEngine *ARXEngineFactory(void);

}
#endif
