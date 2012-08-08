/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "VTCLoopback.h"
#include "IOMXEncoder.h"
#include "IOMXDecoder.h"

#define LOG_NDEBUG 0
#define LOG_TAG "VTC"

using namespace android;


int gFilename = 0;
int gDuration = 10;
int gTestcaseID = 1;
int gPreviewWidth = WIDTH;
int gPreviewHeight = HEIGHT;
int gCameraIndex = 0;
int gCameraFrameRate = 30;
int gNewCameraFrameRate = 0;
int gEnableAlgo = 0;
uint32_t gSliceHeight = 0;
uint32_t gCameraWinX = 50;
uint32_t gCameraWinY = 50;
uint32_t gCameraSurfaceWidth = 400;
uint32_t gCameraSurfaceHeight = 400;
uint32_t gEncoderBitRate = BITRATE;
uint32_t gMinEncoderBitRate = BITRATE;
uint32_t gMaxEncoderBitRate = BITRATE;
uint32_t gDebugFlags = FPS_DECODER; // | ENCODER_ONLY ;
uint32_t gEncoderOutputBufferCount = 4;
uint32_t gEncoderOutputSliceSizeBytes = 0;
uint32_t gEncoderOutputSliceSizeMB = 0;
bool gEnableLoopback = false;
bool gVaryFrameRate = false;
bool gVaryOrientation = false;
char mParamValue[100];
char gRecordFileName[256];
sp<SurfaceComposerClient> gSurfaceComposerClient;
sp<SurfaceControl> gSurfaceControl;
sp<Surface> gPreviewSurface;
sp<ICameraService> gCameraService;
sp<ICamera> gICamera;
sp<MyCameraClient> gCameraClient;
sp<OMXDecoder> mOMXDecoder;
OMX_VIDEO_AVCPROFILETYPE gProfile = OMX_VIDEO_AVCProfileBaseline;
OMX_VIDEO_AVCLEVELTYPE gLevel = OMX_VIDEO_AVCLevel4;
OMX_U32 gRefFrames = 1;

// Add more parameters as needed.
struct Configuration {
    size_t width, height;
};

int test_DEFAULT_Slice();
int test_DEFAULT_Frame();
int test_Robustness();
int test_Frame_Robustness();
int test_Slice_Robustness();

typedef int (*pt2TestFunction)();
pt2TestFunction TestFunctions[10] = {0, test_DEFAULT_Frame, test_DEFAULT_Slice, test_Frame_Robustness, test_Slice_Robustness, 0, 0, 0, 0, 0};


static void PrintCameraFPS() {
    static int mFrameCount = 0;
    static int mLastFrameCount = 0;
    static nsecs_t mLastFpsTime = 0;
    static float mFps = 0;
    mFrameCount++;
    if (!(mFrameCount & 0x1F)) {
        nsecs_t now = systemTime();
        nsecs_t diff = now - mLastFpsTime;
        mFps = ((mFrameCount - mLastFrameCount) * float(s2ns(1))) / diff;
        mLastFpsTime = now;
        mLastFrameCount = mFrameCount;
        VTC_LOGD("Camera: %d Frames, %f FPS", mFrameCount, mFps);
    }
    // XXX: mFPS has the value we want
}

void dump_video_port_values(OMX_PARAM_PORTDEFINITIONTYPE& def) {
  OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def.format.video;

  VTC_LOGD("--------------------------------------------------------------------------------");
  VTC_LOGD("nPortIndex:%d eDir:%s\n",(int)def.nPortIndex,(def.eDir == OMX_DirInput)?"OMX_DirInput":"OMX_DirOutput");
  VTC_LOGD("BufferCountActual:%d BufferCountMin:%d nBufferSize:%d\n", (int)def.nBufferCountActual, (int)def.nBufferCountMin, (int)def.nBufferSize);
  VTC_LOGD("bEnabled:%s bPopulated:%s eDomain:%s BuffersContiguous:%s BufferAlignment:%d",
       (def.bEnabled)?"TRUE":"FALSE",(def.bPopulated)?"TRUE":"FALSE",
       (def.eDomain == OMX_PortDomainVideo)?"Video":"Not Video",
       (def.bBuffersContiguous)?"TRUE":"FALSE",(int)def.nBufferAlignment);

  VTC_LOGD("cMIMEType:%s pNativeRender:%p\n",(def.format.video.cMIMEType)?(def.format.video.cMIMEType):"NULL",def.format.video.pNativeRender);
  VTC_LOGD("nFrameWidth:%d nFrameHeight:%d nStride:%d nSliceHeight:%d",
       (int)def.format.video.nFrameWidth, (int)def.format.video.nFrameHeight, (int)def.format.video.nStride, (int)def.format.video.nSliceHeight);
  VTC_LOGD("nBitrate:%d xFramerate:%d bFlagErrorConcealment:%s",
       (int)def.format.video.nBitrate, (int)def.format.video.xFramerate, (def.format.video.bFlagErrorConcealment)?"TRUE":"FALSE");
  VTC_LOGD("eCompressionFormat:0x%x eColorFormat:0x%x pNativeWindow:%p",
       (def.format.video.eCompressionFormat),
       (def.format.video.eColorFormat), def.format.video.pNativeWindow);
  VTC_LOGD("--------------------------------------------------------------------------------");
}

#define NAME(n) case n: return #n
const char *OMXStateName(OMX_STATETYPE state) {
    switch (state) {
        NAME(OMX_StateInvalid);
        NAME(OMX_StateLoaded);
        NAME(OMX_StateIdle);
        NAME(OMX_StateExecuting);
        NAME(OMX_StatePause);
        NAME(OMX_StateWaitForResources);
        NAME(OMX_StateKhronosExtensions);
        NAME(OMX_StateVendorStartUnused);
        default: return "???";
    }
}


MyCameraClient::MyCameraClient() {
    encoder_is_ready = 0;
    cameraPayloadWaitFlag = 0;
}

void MyCameraClient::dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& data) {
    //VTC_LOGV("=============================================dataCallbackTimestamp");
    CHECK(data != NULL && data->size() > 0);
    if (msgType == CAMERA_MSG_VIDEO_FRAME) {
        if ((gSliceHeight == 0) && (encoder_is_ready)) { // non tunnel mode
            putCameraPayload(data,(int64_t)timestamp/1000);
        } else {
            if (mReleaser != NULL) {
                mReleaser->releaseRecordingFrame(data);
                //VTC_LOGV("CAMERA_MSG_VIDEO_FRAME %p released",data->pointer());
            }
        }
    }
}

void MyCameraClient::putCameraPayload(sp<IMemory> payload, int64_t frameTime) {
    if (gDebugFlags & FPS_CAMERA) PrintCameraFPS();
    if (gDebugFlags & DEBUG_DUMP_CAMERA_TIMESTAMP) VTC_LOGD("CAM TS: %lld", frameTime);

    Mutex::Autolock autoLock(cameraPayloadQueueMutex);
    cameraPayloadQueue.push_back(payload);
    frameTimeQueue.push_back(frameTime);
    if (cameraPayloadQueue.size() >= 1) {
        if ( cameraPayloadWaitFlag ) cameraPayloadWait.signal();
    }
    return;
}

sp<IMemory> MyCameraClient::getCameraPayload(int64_t& frameTime) {
    if (!encoder_is_ready) {
        frameTime = 0;
        VTC_LOGE("getCameraPayload --- returning null");
        return NULL;
    }

    {
        status_t retval;
        Mutex::Autolock autoLock(cameraPayloadQueueMutex);
        if (cameraPayloadQueue.empty()) {
            cameraPayloadWaitFlag = 1;
            retval = cameraPayloadWait.waitRelative(cameraPayloadQueueMutex, TWO_SECOND);
            if (retval || !encoder_is_ready) {
                VTC_LOGD("$$$$$$$$$$$$$$$$$$$$$$$$$$$$ getCameraPayload timed out or we are stopping $$$$$$$$$$$$$$$$$$$$$$$$$$$$");
                frameTime = 0;
                return NULL;
            }
            cameraPayloadWaitFlag = 0;
        }
    }

    List< sp<IMemory> >::iterator iterPayload;
    sp<IMemory> payload;

    List<int64_t>::iterator iterTime;
    int64_t time;

    {
        Mutex::Autolock autoLock(cameraPayloadQueueMutex);

        iterPayload = cameraPayloadQueue.begin();
        payload = (sp<IMemory>)*iterPayload;
        cameraPayloadQueue.erase(iterPayload);

        iterTime = frameTimeQueue.begin();
        time = (int64_t)*iterTime;
        frameTimeQueue.erase(iterTime);
    }

    frameTime = time;
    //VTC_LOGV("%s: pointer[%p], size[%d]", __FUNCTION__, payload->pointer(), payload->size());
    return payload;
}



int createPreviewSurface() {

    gSurfaceComposerClient = new SurfaceComposerClient();
    CHECK_EQ(gSurfaceComposerClient->initCheck(), (status_t)OK);

    gCameraSurfaceWidth = gSurfaceComposerClient->getDisplayWidth(0);
    gCameraSurfaceHeight = gSurfaceComposerClient->getDisplayHeight(0);

    gSurfaceControl = gSurfaceComposerClient->createSurface(0,
            gCameraSurfaceWidth,
            gCameraSurfaceHeight,
            HAL_PIXEL_FORMAT_RGB_565);

    gPreviewSurface = gSurfaceControl->getSurface();

    gSurfaceComposerClient->openGlobalTransaction();
    gSurfaceControl->setLayer(0x7ffffff0);
    gSurfaceControl->setPosition(gCameraWinX, gCameraWinY);
    gSurfaceControl->setSize(gCameraSurfaceWidth, gCameraSurfaceHeight);
    gSurfaceControl->show();
    gSurfaceComposerClient->closeGlobalTransaction();

    return 0;
}

int destroyPreviewSurface() {

    if ( NULL != gPreviewSurface.get() ) {
        gPreviewSurface.clear();
    }

    if ( NULL != gSurfaceControl.get() ) {
        gSurfaceControl->clear();
        gSurfaceControl.clear();
    }

    if ( NULL != gSurfaceComposerClient.get() ) {
        gSurfaceComposerClient->dispose();
        gSurfaceComposerClient.clear();
    }

    return 0;
}


int configureCamera() {

    createPreviewSurface();

    sp<IServiceManager> sm = defaultServiceManager();
    CHECK(sm.get() != NULL);

    sp<IBinder> binder = sm->getService(String16("media.camera"));
    CHECK(binder.get() != NULL);

    gCameraService = interface_cast<ICameraService>(binder);
    CHECK(gCameraService.get() != NULL);

    gCameraClient = new MyCameraClient();
    int i = 0;
    do {
        gICamera = gCameraService->connect(gCameraClient, gCameraIndex);
        if (gICamera.get() != NULL) break;
        VTC_LOGD("\n\n\n========= Camera Busy. So relax and retry. ===========\n\n\n");
        sleep(1);
        i++;
    } while (i < 10);

    if (gICamera == NULL) return -1;

    VTC_LOGD("Acquired Camera");
    gICamera->setPreviewDisplay(gPreviewSurface);
    gCameraClient->setReleaser(gICamera.get());

    String8 param_str = gICamera->getParameters();
    CameraParameters params(param_str);
    params.setPreviewSize(gPreviewWidth, gPreviewHeight);
    params.setPreviewFrameRate(gCameraFrameRate);
    params.set(CameraParameters::KEY_RECORDING_HINT, CameraParameters::TRUE);// Set recording hint, otherwise it defaults to high-quality and there is not enough memory available to do playback and camera preview simultaneously!
    params.set("internal-vtc-hint", CameraParameters::TRUE);

    if (gEnableAlgo & 1) { //VNF Enabled based on cmd line
        VTC_LOGD("VNF is enabled!!! \n");
        params.set("vnf", CameraParameters::TRUE);
    } else {
        VTC_LOGD("VNF is disabled!!! \n");
        params.set("vnf", CameraParameters::FALSE);
    }

    sprintf(mParamValue,"%u,%u", gCameraFrameRate*1000, gCameraFrameRate*1000);
    params.set("preview-fps-range", mParamValue);
    params.set("mode","video-mode");

    if ((!gSliceHeight) && (gEnableAlgo & 2)) { // VSTAB not supported for Slice Mode
        VTC_LOGD("VSTAB is enabled!!! \n");
        params.set("vstab",CameraParameters::TRUE);
        params.set("video-stabilization",CameraParameters::TRUE);
    } else {
        VTC_LOGD("VSTAB is disabled!!! \n");
        params.set("vstab",CameraParameters::FALSE);
        params.set("video-stabilization",CameraParameters::FALSE);
    }

    //params.dump();
    gICamera->setParameters(params.flatten());
    return 0;
}

void stopPreview() {
    gICamera->stopPreview();
    gCameraClient->setReleaser(NULL);
    gICamera->disconnect();
    gICamera.clear();
    gCameraClient.clear();
    gCameraService.clear();
    destroyPreviewSurface();
}

void encodedBufferCallback(void* pBuffer, OMX_U32 nFilledLen, OMX_TICKS nTimeStamp) {
    if (mOMXDecoder.get()) {
        mOMXDecoder->AcceptEncodedBuffer(pBuffer, nFilledLen, nTimeStamp);
    } else {
        VTC_LOGE("\n\nDECODER IS NULL !!!!!!! \n\n");
    }
}

void setFrameRate(sp<OMXEncoder> pOMXEncoder) {
    sleep(8);
    pOMXEncoder->mDebugFlags |= FPS_ENCODER;

    // Changing the framerate in camera.
    String8 param_str = gICamera->getParameters();
    CameraParameters params(param_str);
    VTC_LOGD("Setting new framerate: %d", gNewCameraFrameRate);
    params.setPreviewFrameRate(gNewCameraFrameRate);
    sprintf(mParamValue,"%u,%u", gNewCameraFrameRate*1000, gNewCameraFrameRate*1000);
    params.set("preview-fps-range", mParamValue);
    gICamera->setParameters(params.flatten());

    // Changing the framerate in encoder.
    pOMXEncoder->changeFrameRate(gNewCameraFrameRate);
}

void varyBitRate1(sp<OMXEncoder> pOMXEncoder) {
    uint32_t currentBitRate = gMinEncoderBitRate;
    uint32_t step = (gMaxEncoderBitRate - gMinEncoderBitRate) / 10;
    while (currentBitRate <= gMaxEncoderBitRate) {
        sleep(10);
        pOMXEncoder->changeBitRate(currentBitRate);
        currentBitRate += step;
    }
}


void varyBitRate2(sp<OMXEncoder> pOMXEncoder) {
    int32_t step = gMaxEncoderBitRate - gMinEncoderBitRate;
    uint32_t currentBitRate = gMinEncoderBitRate;

    int i = 1;
    while (i <= 10) {
        sleep(10);
        pOMXEncoder->changeBitRate(currentBitRate);
        currentBitRate += step;
        step *= -1;
        i++;
    }
}


void varyBitRate(sp<OMXEncoder> pOMXEncoder) {
    if ((gMinEncoderBitRate == 0) || (gMaxEncoderBitRate == 0)) return;

    pOMXEncoder->mDebugFlags |= ENCODER_EFFECTIVE_BITRATE;

    varyBitRate1(pOMXEncoder);
    varyBitRate2(pOMXEncoder);
}

void varyFrameRate(sp<OMXEncoder> pOMXEncoder) {
    for (int i = 7; i <= 30; i+=2) {
        gNewCameraFrameRate = i;
        setFrameRate(pOMXEncoder);
    }
    for (int i = 30; i >= 7; i-=2) {
        gNewCameraFrameRate = i;
        setFrameRate(pOMXEncoder);
    }
}

void varyOrientation(sp<OMXEncoder> pOMXEncoder) {
    //TODO : Check on the requirement and enhance to support this functionality
}


int test_DEFAULT_Frame() {
    status_t err = 0;

    if (gEnableLoopback) {
        mOMXDecoder = new OMXDecoder(gPreviewWidth, gPreviewHeight, gCameraFrameRate);
        mOMXDecoder->mDebugFlags = gDebugFlags;
        err = mOMXDecoder->configure(gProfile, gLevel, gRefFrames);
        if (err != 0) return -1;
        err = mOMXDecoder->prepare();
        if (err != 0) return -1;
    }

    configureCamera();

    OMXClient omxclient;
    CHECK_EQ(omxclient.connect(), (status_t)OK);
    sp<IOMX> omx = omxclient.interface();
    IOMX::node_id node = 0;
    sp<OMXEncoderObserver> observer = new OMXEncoderObserver();
    err = omx->allocateNode("OMX.TI.DUCATI1.VIDEO.H264E", observer, &node);
    if (err != OK) {
        VTC_LOGD("Failed to allocate OMX node!!");
        return -1;
    }

    sp<OMXEncoder> pOMXEncoder = new OMXEncoder(omx, node, gCameraClient,
            gPreviewWidth, gPreviewHeight, gCameraFrameRate, gEncoderBitRate, gRecordFileName, 0 /*SliceHeight*/);
    observer->setCodec(pOMXEncoder);
    pOMXEncoder->mDebugFlags = gDebugFlags;
    pOMXEncoder->mOutputBufferCount = gEncoderOutputBufferCount;

    if (gEnableLoopback) pOMXEncoder->setCallback(&encodedBufferCallback);

    err = pOMXEncoder->configure(gProfile, gLevel, gRefFrames);
    if (err != 0) return -1;
    err = pOMXEncoder->prepare();
    if (err != 0) return -1;

    gICamera->startPreview();
    sleep(SLEEP_AFTER_STARTING_PREVIEW);

    gICamera->startRecording();
    err = pOMXEncoder->start();
    if (err != 0) return -1;

    if (gEnableLoopback) {
        err = mOMXDecoder->start();
        if (err != 0) return -1;
    }


    if (gNewCameraFrameRate) setFrameRate(pOMXEncoder);
    if (gMinEncoderBitRate != gMaxEncoderBitRate) varyBitRate(pOMXEncoder);
    if (gVaryFrameRate) varyFrameRate(pOMXEncoder);
    if (gVaryOrientation) varyOrientation(pOMXEncoder);

    sleep(gDuration);

    gICamera->stopRecording();
    pOMXEncoder->stop();
    pOMXEncoder->deinit();
    stopPreview();

    if (gEnableLoopback) {
        mOMXDecoder->stop();
        mOMXDecoder.clear();
    }
    pOMXEncoder.clear();
    observer.clear();

    return 0;
}

int test_DEFAULT_Slice() {
    status_t err = 0;
    if (gSliceHeight == 0) gSliceHeight = gPreviewHeight / 2;
    if (gSliceHeight < 128) gSliceHeight = gPreviewHeight;

    if (gEnableLoopback) {
        mOMXDecoder = new OMXDecoder(gPreviewWidth, gPreviewHeight, gCameraFrameRate);
        if (gEncoderOutputSliceSizeBytes || gEncoderOutputSliceSizeMB) {
            mOMXDecoder->mDebugFlags = gDebugFlags | INPUT_OUTPUT_SLICE_MODE;
        } else {
            mOMXDecoder->mDebugFlags = gDebugFlags | INPUT_SLICE_MODE;
        }
        err = mOMXDecoder->configure(gProfile, gLevel, gRefFrames);
        if (err != 0) return -1;
        err = mOMXDecoder->prepare();
        if (err != 0) return -1;
    }

    configureCamera();

    OMXClient omxclient;
    CHECK_EQ(omxclient.connect(), (status_t)OK);
    sp<IOMX> omx = omxclient.interface();
    IOMX::node_id node = 0;
    sp<OMXEncoderObserver> observer = new OMXEncoderObserver();
    err = omx->allocateNode("OMX.TI.DUCATI1.VIDEO.H264E", observer, &node);
    if (err != OK) {
        VTC_LOGD("Failed to allocate OMX node!!");
        return -1;
    }

    sp<OMXEncoder> pOMXEncoder = new OMXEncoder(omx, node, gCameraClient, gPreviewWidth, gPreviewHeight, gCameraFrameRate, gEncoderBitRate, gRecordFileName, gSliceHeight);
    observer->setCodec(pOMXEncoder);
    pOMXEncoder->mDebugFlags = gDebugFlags;
    if (gEnableLoopback) pOMXEncoder->setCallback(&encodedBufferCallback);
    err = pOMXEncoder->configure(gProfile, gLevel, gRefFrames);
    if (err != 0) return -1;

    OMX_TI_COMPONENT_HANDLE compHandle;
    INIT_OMX_STRUCT(&compHandle, OMX_TI_COMPONENT_HANDLE);
    err = omx->getParameter(node, (OMX_INDEXTYPE)OMX_TI_IndexComponentHandle, &compHandle, sizeof(compHandle));
    if (err != OK) {
        VTC_LOGD("get OMX_TI_IndexComponentHandle failed : %d", err);
        return -1;
    }

    // Set up the Tunnel
    String8 param_str = gICamera->getParameters();
    CameraParameters params(param_str);
    sprintf(mParamValue,"%u", gSliceHeight);
    params.set("encoder_slice_height", mParamValue);
    sprintf(mParamValue,"%u", (int)compHandle.pHandle);
    params.set("encoder_handle", mParamValue);
    gICamera->setParameters(params.flatten());

    err = pOMXEncoder->prepare();
    if (err != 0) return -1;

    gICamera->sendCommand(CAMERA_CMD_PREVIEW_INITIALIZATION, 0, 0);

    if (gEncoderOutputSliceSizeBytes || gEncoderOutputSliceSizeMB) {
        sleep(SLEEP_AFTER_STARTING_PREVIEW);
        //Setting the Encoder output slice mode
        err = pOMXEncoder->setEncoderOutputSlice(gPreviewHeight, gPreviewWidth, gEncoderOutputSliceSizeBytes, gEncoderOutputSliceSizeMB);
        if (err != 0) {
            VTC_LOGD("pOMXEncoder->setEncoderOutputSlice error \n");
            return -1;
        }
    }

    //Moving to Executing State
    err = pOMXEncoder->start();
    if (err != 0) return -1;

    err = gICamera->startPreview();
    if (err != 0) return -1;

    if (gEnableLoopback) err = mOMXDecoder->start();
    if (err != 0) return -1;


    if (gNewCameraFrameRate) setFrameRate(pOMXEncoder);
    if (gMinEncoderBitRate != gMaxEncoderBitRate) varyBitRate(pOMXEncoder);
    if (gVaryFrameRate) varyFrameRate(pOMXEncoder);


    sleep(gDuration);
    //camera goes to idle
    gICamera->sendCommand(CAMERA_CMD_PREVIEW_DEINITIALIZATION, 0, 0);
    //encoder goes to idle
    pOMXEncoder->stop();
    //delay for state changes
    usleep (100000);
    //camera goes to loaded
    stopPreview();
    //encoder goes to loaded
    pOMXEncoder->deinit();

    if (gEnableLoopback) {
        mOMXDecoder->stop();
        mOMXDecoder.clear();
    }
    pOMXEncoder.clear();
    //observer.clear();

    return 0;
}

// TODO: One should not be required to release the camera and destroy the node
// everytime we start and stop or change resolution or some other parameter.
// Presently, this code does not work as is...
int test_Robustness() {
    status_t err = 0;
    int loopCnt = 0;
    gDuration = 5;

    configureCamera();
    gICamera->startPreview();
    sleep(SLEEP_AFTER_STARTING_PREVIEW);

    OMXClient omxclient;
    CHECK_EQ(omxclient.connect(), (status_t)OK);
    sp<IOMX> omx = omxclient.interface();
    IOMX::node_id node = 0;
    sp<OMXEncoderObserver> observer = new OMXEncoderObserver();
    err = omx->allocateNode("OMX.TI.DUCATI1.VIDEO.H264E", observer, &node);
    if (err != OK) {
        VTC_LOGD("Failed to allocate OMX node!!");
        return -1;
    }

    sp<OMXEncoder> pOMXEncoder = new OMXEncoder(omx, node, gCameraClient, gPreviewWidth, gPreviewHeight, gCameraFrameRate, gEncoderBitRate, gRecordFileName, 0 /*SliceHeight*/);
    observer->setCodec(pOMXEncoder);


    while(loopCnt < 3) {
        sleep(SLEEP_AFTER_STARTING_PREVIEW);

        pOMXEncoder->resetParameters(gPreviewWidth, gPreviewHeight, gCameraFrameRate, gEncoderBitRate, gRecordFileName, 0 /*SliceHeight*/);
        pOMXEncoder->mDebugFlags = gDebugFlags;
        pOMXEncoder->mOutputBufferCount = gEncoderOutputBufferCount;

        if (gEnableLoopback) pOMXEncoder->setCallback(&encodedBufferCallback);

        err = pOMXEncoder->configure(gProfile, gLevel, gRefFrames);
        if (err != 0) return -1;
        err = pOMXEncoder->prepare();
        if (err != 0) return -1;


        gICamera->startRecording();
        err = pOMXEncoder->start();
        if (err != 0) return -1;


        sleep(gDuration);

        gICamera->stopRecording();
        pOMXEncoder->stop();
        pOMXEncoder->deinit();

        loopCnt++;
        VTC_LOGD("#######################################################################");
        VTC_LOGD("#######################################################################");
    }

    stopPreview();

    return 0;
}

// TODO: Combine these 2 tests into one. Reuse.
int test_Frame_Robustness() {
    const Configuration configdata[] = {
        { 160, 120 },
        { 320, 240 },
        { 640, 480 },
        { 1280, 720 },
    };

    for (int i = 0, j = 0; i < 4000; i++) {
        gPreviewWidth = configdata[j].width;
        gPreviewHeight = configdata[j].height;
        VTC_LOGD("##################################################################");
        VTC_LOGD("#####################  ITERATION %d : %d x %d ###################", i, gPreviewWidth, gPreviewHeight);
        VTC_LOGD("##################################################################");
        sleep(1);

        test_DEFAULT_Frame();
        j++;
        j = j % (sizeof(configdata)/sizeof(Configuration));
    }
    return 0;

}

int test_Slice_Robustness() {
    const Configuration configdata[] = {
//        { 160, 120 }, // If this resolution is included, it results in random errors being thrown. Recall that the min slice height is 128 which is more than 120.
        { 320, 240 },
        { 640, 480 },
        { 1280, 720 },
    };

    for (int i = 0, j = 0; i < 4000; i++) {
        gPreviewWidth = configdata[j].width;
        gPreviewHeight = configdata[j].height;
        VTC_LOGD("##################################################################");
        VTC_LOGD("#####################  ITERATION %d : %d x %d ###################", i, gPreviewWidth, gPreviewHeight);
        VTC_LOGD("##################################################################");
        sleep(1);

        test_DEFAULT_Slice();
        j++;
        j = j % (sizeof(configdata)/sizeof(Configuration));
    }
    return 0;

}


void printUsage() {
    printf("\n\nApplication for testing VTC using IOMX");
    printf("\n\nIn this application:");
    printf("\n\t- Camera Preview is started.");
    printf("\n\t- Preview Frames are sent to encoder for encoding.");
    printf("\n\t- The encoded frames are then sent to the decoder for decoding.");
    printf("\n\t- The decoded frames are then rendered along side the preview window.");
    printf("\n\nThe application can be run in frame mode or slice mode.");

    printf("\n\n\nUsage: /system/bin/VTCLoopbackTest <Testcase_ID> <options>\n");

    printf("\n\n\nTest Case ID can be any of the following");
    printf("\n1 - Default test case. Frame Mode. No Loopback. Does not require any parameters to be set.");
    printf("\n2 - Slice Mode. No Loopback. Does not require any parameters to be set.");
    printf("\n3 - Test Robustness. Frame Mode.");
    printf("\n4 - Test Robustness. Slice Mode.");

    printf("\n\n\nAvailable Options:");
    printf("\n-t: Test case ID. Default = %d", gTestcaseID);
    printf("\n-n: Record Filename(/mnt/sdcard/video_0.264) is appended with this number. Default = %d", gFilename);
    printf("\n-w: Preview/Record Width. Default = %d", gPreviewWidth);
    printf("\n-e: Preview/Record Height. Default = %d", gPreviewHeight);
    printf("\n-d: Recording time in secs. Default = %d seconds", gDuration);
    printf("\n-f: Framerate. Default = %d", gCameraFrameRate);
    printf("\n-b: Bitrate. Default = %d", gEncoderBitRate);
    printf("\n-s: Slice Height in # of lines. Default = %d", gSliceHeight);
    printf("\n-g: Debug Options. Refer to source for usage.");
    printf("\n-c: Camera Index. Default = %d", gCameraIndex);
    printf("\n-p: Print FPS. 4 = dont write to file. Print Encoder FPS");
    printf("\n-o: Encoder Output Buffer Count. Default = %d", gEncoderOutputBufferCount);
    printf("\n-l: Enable loopback. Default = %d", gEnableLoopback);
    printf("\n-j: Change to this framerate at runtime after 8 seconds. Default = %d", gNewCameraFrameRate);
    printf("\n-v: VNF/VSTAB Option. 1-Enable VNF, 2-Enable VSTAB, 3-Enable Both, Default = %d", gEnableAlgo);
    printf("\n-i: Min bitrate. Vary the bitrate at run time between min and max values");
    printf("\n-a: Max bitrate. Vary the bitrate at run time between min and max values");
    printf("\n-x: Vary framerate between 7 and 30 at run time");
    printf("\n-r: Test Rotation. Not supported yet.");
    printf("\n-h: Print help menu");

    printf("\n\n\nSample Commands:");
    printf("\n\nTesting Frame mode. Loopback. 720p");
    printf("\nVTCLoopbackTest -t 1 -d 10 -w 1280 -e 720 -b 2000000 -l 1");
    printf("\n\nTesting Slice mode. Loopback. 720p");
    printf("\nVTCLoopbackTest -t 2 -d 10 -w 1280 -e 720 -b 2000000 -l 1");
    printf("\n\nTesting Robustness. Frame mode.");
    printf("\nVTCLoopbackTest -t 3 -l 1");
    printf("\n\nTesting Robustness. Slice mode.");
    printf("\nVTCLoopbackTest -t 4 -l 1");
    printf("\n\nVary Framerate at runtime between 7 and 30");
    printf("\nVTCLoopbackTest -t 1 -x 1 -g 2");
    printf("\n\nTesting Bitrate");
    printf("\nVTCLoopbackTest -t 1 -w 1280 -e 720 -b 2000000 -i 1000000 -a 3000000 -g 64");
    printf("\n\n\n");

}

int main (int argc, char* argv[]) {
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    int opt;
    const char* const short_options = "a:g:n:w:e:d:b:f:s:c:p:t:o:y:m:l:j:i:v:x:r:h";
    const struct option long_options[] = {
        {"debug_flags", 1, NULL, 'g'},
        {"record_filename", 1, NULL, 'n'},
        {"width", 1, NULL, 'w'},
        {"height", 1, NULL, 'e'},
        {"gDuration", 1, NULL, 'd'},
        {"bitrate", 1, NULL, 'b'},
        {"min_bitrate", 1, NULL, 'i'},
        {"max_bitrate", 1, NULL, 'a'},
        {"framerate", 1, NULL, 'f'},
        {"sliceheight", 1, NULL, 's'},
        {"camera_index", 1, NULL, 'c'},
        {"print_fps", 1, NULL, 'p'},
        {"testcase_id", 1, NULL, 't'},
        {"out_buff_cnt", 1, NULL, 'o'},
        {"encoder_slicesize_bytes", 1, NULL, 'y'},
        {"encoder_slicesize_mb", 1, NULL, 'm'},
        {"enable_loopback", 1, NULL, 'l'},
        {"new_framerate", 1, NULL, 'j'},
        {"vary_framerate", 1, NULL, 'x'},
        {"vary_rotation", 1, NULL, 'r'},
        {"algo", 1, NULL, 'v'},
        {"help", 1, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    while((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch(opt) {
            case 'h':
                printUsage();
                return 0;
            case 'g':
                gDebugFlags = atoi(optarg);
                break;
            case 'l':
                gEnableLoopback = atoi(optarg);
                break;
            case 'x':
                gVaryFrameRate = atoi(optarg);
                break;
            case 'r':
                gVaryOrientation = atoi(optarg);
                break;
            case 'o':
                gEncoderOutputBufferCount = atoi(optarg);
                if (gEncoderOutputBufferCount > ENCODER_MAX_BUFFER_COUNT)
                    gEncoderOutputBufferCount = ENCODER_MAX_BUFFER_COUNT;
                break;
            case 'y':
                gEncoderOutputSliceSizeBytes = atoi(optarg);
                break;
            case 'm':
                gEncoderOutputSliceSizeMB = atoi(optarg);
                break;
            case 't':
                gTestcaseID = atoi(optarg);
                break;
            case 'n':
                gFilename = atoi(optarg);
                break;
            case 'w':
                gPreviewWidth = atoi(optarg);
                break;
            case 'e':
                gPreviewHeight = atoi(optarg);
                break;
            case 'd':
                gDuration = atoi(optarg);
                break;
            case 'b':
                gEncoderBitRate = atoi(optarg);
                break;
            case 'i':
                gMinEncoderBitRate = atoi(optarg);
                break;
            case 'a':
                gMaxEncoderBitRate = atoi(optarg);
                break;
            case 'f':
                gCameraFrameRate = atoi(optarg);
                break;
            case 'j':
                gNewCameraFrameRate = atoi(optarg);
                break;
            case 's':
                gSliceHeight = atoi(optarg);
                break;
            case 'c':
                gCameraIndex = atoi(optarg);
                break;
            case 'v':
                gEnableAlgo = atoi(optarg);
                break;
            case ':':
                VTC_LOGE("\nError - Option `%c' needs a value\n\n", optopt);
                return -1;
            case '?':
                VTC_LOGE("\nError - No such option: `%c'\n\n", optopt);
                return -1;
        }
    }

    if (argc == 1) printUsage();
    if (gVaryFrameRate) gCameraFrameRate = 30; // Framerate must be initialized to 30.

    sprintf(gRecordFileName,  "/mnt/sdcard/video_%d.264", gFilename);
    VTC_LOGI("\n\nRecorded Output is stored in %s\n\n", gRecordFileName);

    system("setprop debug.vfr.enable 0");

    TestFunctions[gTestcaseID]();
    return 0;
}

