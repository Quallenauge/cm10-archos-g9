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


#define LOG_NDEBUG 0
#define LOG_TAG "CamHalTest"

#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>


#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <camera/Camera.h>
#include <camera/ICamera.h>

#include <cutils/log.h>
#include <media/stagefright/CameraSource.h>
#include <media/stagefright/MetaData.h>

#ifdef ANDROID_API_JB_OR_LATER
#include <gui/Surface.h>
#include <gui/ISurface.h>
#include <gui/ISurfaceComposer.h>
#include <gui/ISurfaceComposerClient.h>
#include <gui/SurfaceComposerClient.h>
#else
#include <surfaceflinger/Surface.h>
#include <surfaceflinger/ISurface.h>
#include <surfaceflinger/ISurfaceComposer.h>
#include <surfaceflinger/ISurfaceComposerClient.h>
#include <surfaceflinger/SurfaceComposerClient.h>
#endif

#include <cutils/properties.h>

#include <hardware/camera.h>
#include <hardware/hardware.h>
#include "CameraHardwareInterface.h"

#include "VtcCommon.h"

using namespace android;

int startPreviewNow = 1;
int mWidth = 640;
int mHeight = 480;

static int cameraId = 0;
static int mPriority = 0;
camera_module_t *mModule;
sp<CameraHardwareInterface> mHardware = NULL;

static int surface_setup_complete = 0;
sp<Surface> mSurface;
sp<SurfaceComposerClient> mComposerClient;
sp<SurfaceControl> mSurfaceControl;
sp<ANativeWindow> mWindow;

static pthread_cond_t mCond;
static pthread_mutex_t mMutex;


void my_notifyCallback(int32_t msgType, int32_t ext1, int32_t ext2, void* user) {
    VTC_LOGD("\n\nnotifyCallback(%d)\n\n", msgType);
}

void my_dataCallback(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata, void* user) {
    VTC_LOGD("\n\ndataCallback(%d)\n\n", msgType);
}

void my_dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr, void* user) {
    VTC_LOGD("\n\ndataCallbackTimestamp(%d)\n\n", msgType);
}

void surfaceInit() {
    status_t retval;

    mComposerClient = new SurfaceComposerClient;

    retval = mComposerClient->initCheck();
    if (retval != NO_ERROR) {
        VTC_LOGE("mCOmposerClient->initCheck failed");
        return;
    }

    mSurfaceControl = mComposerClient->createSurface(String8("NativeSurface"),
            0, mWidth, mHeight,
            PIXEL_FORMAT_OPAQUE, ISurfaceComposer::eFXSurfaceNormal);

    if (mSurfaceControl == NULL) {
        VTC_LOGE("mComposerClient->createSurface failed");
        return;
    }

    if (mSurfaceControl->isValid()) {
        VTC_LOGE("mSurfaceControl is valid");
    } else {
        VTC_LOGE("mSurfaceControl is not valid");
        return;
    }

    SurfaceComposerClient::openGlobalTransaction();

    retval =  mSurfaceControl->setPosition(100,100);
    if (retval != NO_ERROR) {
        VTC_LOGE("mCOmposerClient->setPosition failed");
        return;
    }

    retval =  mSurfaceControl->setSize(400,400);
    if (retval != NO_ERROR) {
        VTC_LOGE("mCOmposerClient->setPosition failed");
        return;
    }

    retval =  mSurfaceControl->setLayer(999990);
    if (retval != NO_ERROR) {
        VTC_LOGE("mCOmposerClient->setLayer 999990 failed");
        return;
    }

    retval = mSurfaceControl->show();
    if (retval != NO_ERROR) {
        VTC_LOGE("mCOmposerClient->show failed");
        return;
    }
    SurfaceComposerClient::closeGlobalTransaction();

    mSurface = mSurfaceControl->getSurface();
    if (mSurface == NULL) {
        VTC_LOGE("mSurfaceControl->getSurface failed");
        return;
    }

    surface_setup_complete = 1;
}

int main (int argc, char* argv[]) {
    status_t result;
    char cmd[160],*cmdptr;
    struct camera_info info;
    char camera_device_name[10];

    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    if (argc >= 2) {
        mPriority = atoi(argv[1]);
    }

    if (argc == 3) {
        startPreviewNow = atoi(argv[2]);
    }

    if (hw_get_module(CAMERA_HARDWARE_MODULE_ID,
            (const hw_module_t **)&mModule) < 0) {
        VTC_LOGE("Could not load camera HAL module");
        return -1;
    }
    VTC_LOGD("\nLoaded the camera module\n");


    if (mModule->get_camera_info(cameraId, &info) != OK) {
        VTC_LOGE("Invalid camera id %d", cameraId);
        return -1;
    }
    VTC_LOGD("\nLoaded the camera properties\n");

    if (startPreviewNow == 0) return -1;

    snprintf(camera_device_name, sizeof(camera_device_name), "%d", cameraId);
    mHardware = new CameraHardwareInterface(camera_device_name);
    if (mHardware->initialize(&mModule->common) != OK) {
        mHardware.clear();
        VTC_LOGE("mHardware->initialize FAILED");
        return -1;
    }
    VTC_LOGD("\nInitialized the camera hardware\n");

    mHardware->setCallbacks(my_notifyCallback,
            my_dataCallback,
            my_dataCallbackTimestamp,
            (void *)cameraId);

    mHardware->enableMsgType(CAMERA_MSG_ERROR | CAMERA_MSG_PREVIEW_METADATA);

    String8 param_str(mHardware->getParameters().flatten());
    CameraParameters params(param_str);
    params.setPreviewSize(mWidth, mHeight);
    params.set("priority", mPriority);
    //params.dump();
    mHardware->setParameters(params);

    surfaceInit();
    if (surface_setup_complete == 0) {
        VTC_LOGE("\n\nsurfaceInit failed! \n\n");
        goto EXIT;
    }

    mWindow = mSurface;
    if (mWindow == 0) {
        VTC_LOGE("\n\nWhy is mWindow == 0?? \n\n");
        goto EXIT;
    }

    result = native_window_api_connect(mWindow.get(), NATIVE_WINDOW_API_CAMERA);
    if (result != NO_ERROR) {
        VTC_LOGE("native_window_api_connect failed: %s (%d)", strerror(-result),
                result);
        goto EXIT;
    }

    native_window_set_scaling_mode(mWindow.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    native_window_set_buffers_transform(mWindow.get(), 0);

    result = mHardware->setPreviewWindow(mWindow);
    if (result != NO_ERROR) {
        VTC_LOGE("mHardware->setPreviewWindow");
        goto EXIT;
    }

    mHardware->startPreview();

#if USE_TIMED_WAIT
    pthread_mutex_init(&mMutex, NULL);
    pthread_cond_init(&mCond, NULL);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 30;
    pthread_mutex_lock(&mMutex);
    pthread_cond_timedwait(&mCond, &mMutex, &ts);
    pthread_mutex_unlock(&mMutex);
#else

    while (1) {
        cmdptr = fgets(cmd, sizeof(cmd), stdin);
        if (!strncmp(cmdptr,"quit",4)) {
            break;
        }
    }
#endif
    VTC_LOGD("\n\n STOPPING PREVIEW \n\n");
    mHardware->stopPreview();


EXIT:
    if (mHardware != 0){
        mHardware->release();

        if (mWindow != 0) {
            result = native_window_api_disconnect(mWindow.get(), NATIVE_WINDOW_API_CAMERA);
            if (result != NO_ERROR) {
                VTC_LOGW("native_window_api_disconnect failed: %s (%d)", strerror(-result), result);
            }
            mWindow = 0;
        }
        mHardware.clear();
    }

    if ( NULL != mSurface.get() ) {
        mSurface.clear();
    }

    if ( NULL != mSurfaceControl.get() ) {
        mSurfaceControl->clear();
        mSurfaceControl.clear();
    }

    if ( NULL != mComposerClient.get() ) {
        mComposerClient->dispose();
        mComposerClient.clear();
    }

#if USE_TIMED_WAIT
    pthread_mutex_destroy(&mMutex);
    pthread_cond_destroy(&mCond);
#endif

    VTC_LOGD("\n\n SUCCESS \n\n");

    return 0;
}

