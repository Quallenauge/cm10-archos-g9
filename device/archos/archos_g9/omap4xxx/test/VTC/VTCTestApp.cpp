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
#define LOG_TAG "VTCTest"

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
#include <system/audio.h>

#include <cutils/log.h>
#include <media/mediaplayer.h>
#include <media/mediarecorder.h>
#include <media/stagefright/OMXClient.h>
#include <media/stagefright/OMXCodec.h>
#include <media/stagefright/MediaDefs.h>
#ifdef ANDROID_API_JB_OR_LATER
#include <media/stagefright/foundation/ADebug.h>
#else
#include <media/stagefright/MediaDebug.h>
#endif
#include <media/stagefright/MPEG4Writer.h>
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

#include "OMX_TI_Index.h"
#include "OMX_TI_Video.h"

#include "VtcCommon.h"

#define SLEEP_AFTER_STARTING_PREVIEW 3
#define SLEEP_AFTER_STARTING_PLAYBACK 5
#define WIDTH 1280
#define HEIGHT 720

using namespace android;

int mMediaServerPID = -1;
int mStartMemory = 0;
int mTestCount = 0;
int mFailCount = 0;

int filename = 0;
int mDuration = 10;
int mPlaybackDuration = 0;
int testcase = 1;
int InsertIDRFrameEveryXSecs = 1;
int mPreviewWidth = WIDTH;
int mPreviewHeight = HEIGHT;
int mOutputFd = -1;
int camera_index = 0;
int mSliceSizeBytes = 500;
int mSliceSizeMB = 100;
int mDisable1080pTesting = 0;
int mRobustnessTestType = -1;
bool mIsSizeInBytes = true;
bool mCameraThrewError = false;
bool mMediaPlayerThrewError = false;
char mParamValue[100];
char mRecordFileName[256];
char mPlaybackFileName[256];
FILE *mResultsFP = NULL;
// If seconds <  0, only the first frame is I frame, and rest are all P frames
// If seconds == 0, all frames are encoded as I frames. No P frames
// If seconds >  0, it is the time spacing (seconds) between 2 neighboring I frames
int32_t mIFramesIntervalSec = 1;
uint32_t mVideoBitRate      = 1000000;
uint32_t mVideoFrameRate    = 30;
uint32_t mNewVideoBitRate   = 100000;
uint32_t mNewVideoFrameRate = 15;
uint32_t mCycles = 10;

sp<Camera> camera;
sp<SurfaceComposerClient> client;
sp<SurfaceControl> surfaceControl;
sp<Surface> previewSurface;
CameraParameters params;
sp<MediaRecorder> recorder;
sp<MediaPlayer> player;
sp<SurfaceComposerClient> playbackComposerClient;
sp<SurfaceControl> playbackSurfaceControl;
sp<Surface> playbackSurface;
//To perform better
static pthread_cond_t mCond;
static pthread_mutex_t mMutex;
bool bPlaying = false;
bool bRecording = false;

uint32_t cameraWinX = 0;
uint32_t cameraWinY = 0;
uint32_t playerWinX = 0;
uint32_t playerWinY = 0;

uint32_t cameraSurfaceWidth = 0;
uint32_t cameraSurfaceHeight = 0;
uint32_t playbackSurfaceWidth = 400;
uint32_t playbackSurfaceHeight = 400;

//forward declarations
int test_ALL();
int test_DEFAULT();
int test_InsertIDRFrames();
int test_MaxNALSize();
int test_ChangeBitRate();
int test_ChangeFrameRate();
int test_PlaybackAndRecord_sidebyside();
int test_PlaybackAndRecord_PIP();
int test_PlaybackOnly();
int test_Robust();


typedef int (*pt2TestFunction)();
pt2TestFunction TestFunctions[10] = {
    test_ALL, // 0
    test_DEFAULT, // 1
    test_InsertIDRFrames, // 2
    test_MaxNALSize, // 3
    test_ChangeBitRate, // 4
    test_ChangeFrameRate, // 5
    test_PlaybackAndRecord_sidebyside, // 6
    test_PlaybackAndRecord_PIP, // 7
    test_PlaybackOnly, // 8
    test_Robust // 9
};

class MyCameraListener: public CameraListener {
    public:
        virtual void notify(int32_t msgType, int32_t ext1, int32_t /* ext2 */) {
                        VTC_LOGD("\n\n\n notifynotifynotifynotifynotifynotifynotifyCamera reported an error!!!\n\n\n");

            if ( msgType & CAMERA_MSG_ERROR && (ext1 == 1)) {
                VTC_LOGD("\n\n\n Camera reported an error!!!\n\n\n");
                mCameraThrewError = true;
                pthread_cond_signal(&mCond);
            }
        }
        virtual void postData(int32_t /* msgType */,
                              const sp<IMemory>& /* dataPtr */,
                              camera_frame_metadata_t * /* metadata */){}

        virtual void postDataTimestamp(nsecs_t /* timestamp */, int32_t /* msgType */, const sp<IMemory>& /* dataPtr */){}
};

sp<MyCameraListener> mCameraListener;

class PlayerListener: public MediaPlayerListener {
public:
    virtual void notify(int msg, int ext1, int ext2, const Parcel * /* obj */)
    {
        VTC_LOGD("Notify cb: %d %d %d\n", msg, ext1, ext2);

        if ( msg == MEDIA_PREPARED ){
            VTC_LOGD("MEDIA_PREPARED!");
            player->start();
        }

        if ( msg == MEDIA_SET_VIDEO_SIZE ){
            VTC_LOGD("MEDIA_SET_VIDEO_SIZE!");
        }

        if ( msg == MEDIA_PLAYBACK_COMPLETE ){
            VTC_LOGD("MEDIA_PLAYBACK_COMPLETE!");
            pthread_cond_signal(&mCond);
        }

        if ( msg == MEDIA_ERROR ){
            VTC_LOGD("PLAYER REPORTED MEDIA_ERROR!");
            mMediaPlayerThrewError = true;
            pthread_cond_signal(&mCond);
        }
    }
};

sp<PlayerListener> mPlayerListener;

int getMediaserverInfo(int *PID, int *VSIZE){
    FILE *fp;
    char ps_output[256];

    /* Open the command for reading. */
    fp = popen("ps mediaserver", "r");
    if (fp == NULL) {
        VTC_LOGE("Failed to get mediaserver pid !!!" );
        return -1;
    }

    /* Read the output a line at a time. We need the last line.*/
    while (fgets(ps_output, sizeof(ps_output)-1, fp) != NULL) {}
    char *substring;
    substring = strtok (ps_output," "); // first is the USER name
    substring = strtok (NULL, " "); // second is the PID
    *PID = atoi(substring);
    substring = strtok (NULL, " "); // third is the PPID
    substring = strtok (NULL, " "); // fourth is the VSIZE
    *VSIZE = atoi(substring);
    pclose(fp);
    return 0;
}

int my_pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t * mutex, int waitTimeInMilliSecs) {
    if (waitTimeInMilliSecs == 0)
    {
        return pthread_cond_wait(cond, mutex);
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    if (waitTimeInMilliSecs >= 1000) { // > 1 sec
        ts.tv_sec += (waitTimeInMilliSecs/1000);
    } else {
        ts.tv_nsec += waitTimeInMilliSecs * 1000000;
    }

    return pthread_cond_timedwait(cond, mutex, &ts);
}

int startPlayback() {

    playbackSurfaceControl = playbackComposerClient->createSurface(0, playbackSurfaceWidth, playbackSurfaceHeight, PIXEL_FORMAT_RGB_565);
    CHECK(playbackSurfaceControl != NULL);
    CHECK(playbackSurfaceControl->isValid());

    playbackSurface = playbackSurfaceControl->getSurface();
    CHECK(playbackSurface != NULL);

    playbackComposerClient->openGlobalTransaction();
    playbackSurfaceControl->setLayer(0x7fffffff);
    playbackSurfaceControl->setPosition(playerWinX, playerWinY);
    playbackSurfaceControl->setSize(playbackSurfaceWidth, playbackSurfaceHeight);
    playbackSurfaceControl->show();
    playbackComposerClient->closeGlobalTransaction();

    player = new MediaPlayer();
    mPlayerListener = new PlayerListener();
    mMediaPlayerThrewError = false;
    player->setListener(mPlayerListener);
    player->setDataSource(mPlaybackFileName, NULL);
    player->setVideoSurfaceTexture(playbackSurface->getSurfaceTexture());
    player->prepareAsync();
    bPlaying = true;
    return 0;
}

int stopPlayback() {

    VTC_LOGD("%d: %s", __LINE__, __FUNCTION__);
    player->stop();
    player->setListener(0);
    player->disconnect();
    player.clear();
    mPlayerListener.clear();

    if ( NULL != playbackSurface.get() ) {
        playbackSurface.clear();
    }

    if ( NULL != playbackSurfaceControl.get() ) {
        playbackSurfaceControl->clear();
        playbackSurfaceControl.clear();
    }

    if ( NULL != playbackComposerClient.get() ) {
        playbackComposerClient->dispose();
        playbackComposerClient.clear();
    }

    return 0;
}

int verfiyByPlayback() {
    VTC_LOGD(" ============= verfiyByPlayback ============= ");
    playbackComposerClient = new SurfaceComposerClient();
    CHECK_EQ(playbackComposerClient->initCheck(), (status_t)OK);

    playbackSurfaceWidth = playbackComposerClient->getDisplayWidth(0);
    playbackSurfaceHeight = playbackComposerClient->getDisplayHeight(0);
    VTC_LOGD("Panel WxH = %d x %d", playbackSurfaceWidth, playbackSurfaceHeight);

    startPlayback();
    pthread_mutex_lock(&mMutex);
    if (bPlaying){
        pthread_cond_wait(&mCond, &mMutex);
    }
    pthread_mutex_unlock(&mMutex);
    stopPlayback();
    return 0;
}

int createPreviewSurface() {

    client = new SurfaceComposerClient();
    CHECK_EQ(client->initCheck(), (status_t)OK);

    if ((cameraSurfaceWidth == 0) || (cameraSurfaceHeight == 0)){
        cameraSurfaceWidth = client->getDisplayWidth(0);
        cameraSurfaceHeight = client->getDisplayHeight(0);
    }

    surfaceControl = client->createSurface(0,
            cameraSurfaceWidth,
            cameraSurfaceHeight,
            HAL_PIXEL_FORMAT_RGB_565);

    previewSurface = surfaceControl->getSurface();

    client->openGlobalTransaction();
    surfaceControl->setLayer(0x7fffffff);
    surfaceControl->setPosition(cameraWinX, cameraWinY);
    surfaceControl->setSize(cameraSurfaceWidth, cameraSurfaceHeight);
    surfaceControl->show();
    client->closeGlobalTransaction();

    return 0;
}

int destroyPreviewSurface() {

    if ( NULL != previewSurface.get() ) {
        previewSurface.clear();
    }

    if ( NULL != surfaceControl.get() ) {
        surfaceControl->clear();
        surfaceControl.clear();
    }

    if ( NULL != client.get() ) {
        client->dispose();
        client.clear();
    }

    return 0;
}

int startRecording() {

    if (camera.get() == NULL) return -1;

    recorder = new MediaRecorder();

    if ( NULL == recorder.get() ) {
        VTC_LOGD("Error while creating MediaRecorder\n");
        return -1;
    }

    camera->unlock();

    if ( recorder->setCamera(camera->remote(), camera->getRecordingProxy()) < 0 ) {
        VTC_LOGD("error while setting the camera\n");
        return -1;
    }

    if ( recorder->setVideoSource(1 /*VIDEO_SOURCE_CAMERA*/) < 0 ) {
        VTC_LOGD("error while configuring camera video source\n");
        return -1;
    }

    if ( recorder->setAudioSource(AUDIO_SOURCE_DEFAULT) < 0 ) {
        VTC_LOGD("error while configuring camera audio source\n");
        return -1;
    }

    if ( recorder->setOutputFormat(OUTPUT_FORMAT_THREE_GPP) < 0 ) {
        VTC_LOGD("error while configuring output format\n");
        return -1;
    }

    if(mkdir("/mnt/sdcard/vtc_videos",0777) == -1)
         VTC_LOGD("\n Directory \'vtc_videos\' was not created or maybe it was already created \n");

    mOutputFd = open(mRecordFileName, O_CREAT | O_RDWR);

    if(mOutputFd < 0){
        VTC_LOGD("Error while creating video filename\n");
        return -1;
    }

    if ( recorder->setOutputFile(mOutputFd, 0, 0) < 0 ) {
        VTC_LOGD("error while configuring video filename\n");

        return -1;
    }

    if ( recorder->setVideoFrameRate(mVideoFrameRate) < 0 ) {
        VTC_LOGD("error while configuring video framerate\n");
        return -1;
    }

    if ( recorder->setVideoSize(mPreviewWidth, mPreviewHeight) < 0 ) {
        VTC_LOGD("error while configuring video size\n");
        return -1;
    }

    if ( recorder->setVideoEncoder(VIDEO_ENCODER_H264) < 0 ) {
        VTC_LOGD("error while configuring video codec\n");
        return -1;
    }

    if ( recorder->setAudioEncoder(AUDIO_ENCODER_AMR_NB) < 0 ) {
        VTC_LOGD("error while configuring audio codec\n");
        return -1;
    }

    if ( recorder->setPreviewSurface(previewSurface) < 0 ) {
        VTC_LOGD("error while configuring preview surface\n");
        return -1;
    }

    sprintf(mParamValue,"video-param-encoding-bitrate=%u", mVideoBitRate);
    String8 bit_rate(mParamValue);
    if ( recorder->setParameters(bit_rate) < 0 ) {
        VTC_LOGD("error while configuring bit rate\n");
        return -1;
    }

    sprintf(mParamValue,"video-param-i-frames-interval=%u", mIFramesIntervalSec);
    String8 interval(mParamValue);
    if ( recorder->setParameters(interval) < 0 ) {
        VTC_LOGD("error while configuring i-frame interval\n");
        return -1;
    }

    if ( recorder->prepare() < 0 ) {
        VTC_LOGD("recorder prepare failed\n");
        return -1;
    }

    if ( recorder->start() < 0 ) {
        VTC_LOGD("recorder start failed\n");
        return -1;
    }

    bRecording = true;
    return 0;
}

int stopRecording() {

    VTC_LOGD("stopRecording()");
    if (camera.get() == NULL) return -1;

    if ( NULL == recorder.get() ) {
        VTC_LOGD("invalid recorder reference\n");
        return -1;
    }

    if ( recorder->stop() < 0 ) {
        VTC_LOGD("recorder failed to stop\n");
        return -1;
    }

    recorder->release();
    recorder.clear();

    if ( 0 < mOutputFd ) {
        close(mOutputFd);
    }

    return 0;
}


int startPreview() {
    char value[PROPERTY_VALUE_MAX];
    property_get("disable.VSTAB.VNF", value, "0");
    int disable_VTAB_and_VNF = atoi(value);
    mCameraThrewError = false;
    bRecording = false;

    createPreviewSurface();
    camera = Camera::connect(camera_index);
    if (camera.get() == NULL){
        VTC_LOGE("camera.get() =================== NULL");
        return -1;
    }

    VTC_LOGD("\n\n mPreviewWidth = %d, mPreviewHeight = %d, mVideoFrameRate = %d, mVideoBitRate = %d \n\n\n",
        mPreviewWidth, mPreviewHeight, mVideoFrameRate, mVideoBitRate);

    params.unflatten(camera->getParameters());
    params.setPreviewSize(mPreviewWidth, mPreviewHeight);
    params.setPreviewFrameRate(mVideoFrameRate);
    params.set(CameraParameters::KEY_RECORDING_HINT, CameraParameters::TRUE);// Set recording hint, otherwise it defaults to high-quality and there is not enough memory available to do playback and camera preview simultaneously!
    sprintf(mParamValue,"%u,%u", mVideoFrameRate*1000, mVideoFrameRate*1000);
    params.set("preview-fps-range", mParamValue);

    if(disable_VTAB_and_VNF){
        VTC_LOGI("\n\n\nDisabling VSTAB & VNF (noise reduction)\n\n");
        params.set("vstab" , 0);
        params.set("vnf", 0);
    }

    camera->setParameters(params.flatten());
    camera->setPreviewDisplay(previewSurface);
    mCameraListener = new MyCameraListener();
    camera->setListener(mCameraListener);

    VTC_LOGV("get(preview-fps-range) = %s", params.get("preview-fps-range"));
    VTC_LOGV("get(preview-fps-range-values) = %s", params.get("preview-fps-range-values"));
    VTC_LOGV("get(preview-size-values) = %s\n", params.get("preview-size-values"));
    VTC_LOGV("get(preview-frame-rate-values) = %s", params.get("preview-frame-rate-values"));

    camera->startPreview();
    sleep(SLEEP_AFTER_STARTING_PREVIEW);
    return 0;
}

void stopPreview() {
    if (camera.get() == NULL) return;
    camera->stopPreview();
    camera->disconnect();
    camera.clear();
    mCameraListener.clear();
    destroyPreviewSurface();
}

int test_DEFAULT() {
    startPreview();
    startRecording();
    sleep(mDuration);
    stopRecording();
    stopPreview();
    return 0;
}

int test_InsertIDRFrames() {
    status_t err = 0;
    mIFramesIntervalSec = 0;
    startPreview();
    startRecording();

    int duration = 0;
    while (duration < mDuration) {
        sleep(InsertIDRFrameEveryXSecs);
        duration += InsertIDRFrameEveryXSecs;

        sprintf(mParamValue,"video-param-insert-i-frame=1");
        String8 param(mParamValue);
        err = recorder->setParameters(param);
        if (err != OK) return -1;
        VTC_LOGI("\n Inserted an IDR Frame. \n");
    };

    stopRecording();
    stopPreview();

    return 0;
}


int test_MaxNALSize() {
    status_t err = 0;

    if (mIsSizeInBytes) {
        //Testing size base on bytes
        CHECK(mPreviewWidth > 320);
        CHECK(mSliceSizeBytes >= 256);
    } else {
        //Testing size base on MB
        CHECK(mSliceSizeMB > 6);
        CHECK(mSliceSizeMB < (((mPreviewWidth+15)>> 4) * ((mPreviewHeight+15)>> 4)));
        /* Max # of MB
            1080p=8160
            720p=3600
            VGA=1200
        */
    }

    /* Other limitations:
    4. Input content type should be progressive
    6. Changing parameters at run time will not have effect until next I-frame.
    7. Incase of doing the initial setting of nPFrames = 0 (only initial frame is I-frame and all others P-frames),
        you must request an I-frame to the codec after you have set nSlicesize to see your changes take place.
    */

    mIFramesIntervalSec = 0; //may be needed. dont know for sure. if it is not zero, then in OMXCodec, under support for B Frames, something is done.
    startPreview();
    startRecording();

    if (mIsSizeInBytes) {
        sprintf(mParamValue,"video-param-nalsize-bytes=%u", mSliceSizeBytes);
        String8 param(mParamValue);
        err = recorder->setParameters(param);
        if (err != OK) return -1;
        VTC_LOGI("\n Set the Slice Size in bytes.\n");
    } else {
        sprintf(mParamValue,"video-param-nalsize-macroblocks=%u", mSliceSizeMB);
        String8 param(mParamValue);
        err = recorder->setParameters(param);
        if (err != OK) return -1;
        VTC_LOGI("\n Set the Slice Size in MB\n");
    }

    // Change won't take effect until next IFrame. So, force an IFrame.

    sprintf(mParamValue,"video-param-insert-i-frame=1");
    String8 paramI(mParamValue);
    err = recorder->setParameters(paramI);
    if (err != OK) return -1;
    VTC_LOGI("\n Inserted an IDR Frame. \n");

    sleep(mDuration);
    stopRecording();
    stopPreview();
    mIFramesIntervalSec = 1;
    return 0;
}


int test_ChangeBitRate() {
    startPreview();
    startRecording();
    sleep(mDuration/2);

    sprintf(mParamValue,"video-config-encoding-bitrate=%u", mNewVideoBitRate);
    String8 param(mParamValue);
    status_t err = recorder->setParameters(param);
    if (err != OK) return -1;
    VTC_LOGI("\n\nSet new bitrate. \n\n");

    sleep(mDuration/2);
    stopRecording();
    stopPreview();
    return 0;
}


int test_ChangeFrameRate() {
    startPreview();
    startRecording();
    sleep(mDuration/2);

    // Changing the framerate in camera.
    params.unflatten(camera->getParameters());
    VTC_LOGD("Setting new framerate: %d", mNewVideoFrameRate);
    sprintf(mParamValue,"%u,%u", mNewVideoFrameRate*1000, mNewVideoFrameRate*1000);
    params.set("preview-fps-range", mParamValue);
    VTC_LOGD("get(preview-fps-range) = %s", params.get("preview-fps-range"));
    camera->setParameters(params.flatten());

    // Changing the framerate in encoder.
    sprintf(mParamValue,"video-config-encoding-framerate=%u", mNewVideoFrameRate);
    String8 param(mParamValue);
    status_t err = recorder->setParameters(param);
    if (err != OK) return -1;
    VTC_LOGI("\n\nSet new framerate. \n\n");

    sleep(mDuration/2);
    stopRecording();
    stopPreview();
    return 0;
}

int test_PlaybackAndRecord_sidebyside() {
    playbackComposerClient = new SurfaceComposerClient();
    CHECK_EQ(playbackComposerClient->initCheck(), (status_t)OK);

    int panelwidth = playbackComposerClient->getDisplayWidth(0);
    int panelheight = playbackComposerClient->getDisplayHeight(0);
    VTC_LOGD("Panel WxH = %d x %d", panelwidth, panelheight);
    if (panelwidth < panelheight) {//Portrait Phone
        VTC_LOGD("\nPortrait Device\n");
        playbackSurfaceWidth = panelwidth;
        playbackSurfaceHeight = panelheight/2;
        playerWinX = 0;
        playerWinY = 0;

        cameraWinX = 0;
        cameraWinY = playbackSurfaceHeight;
        cameraSurfaceWidth = panelwidth;
        cameraSurfaceHeight = panelheight/2;
    } else {// Landscape
        VTC_LOGD("\n Landscape Device\n");
        playbackSurfaceWidth = panelwidth/2;
        playbackSurfaceHeight = panelheight;
        playerWinX = 0;
        playerWinY = 0;

        cameraWinX = playbackSurfaceWidth;
        cameraWinY = 0;
        cameraSurfaceWidth = panelwidth/2;
        cameraSurfaceHeight = panelheight;
    }

    startPlayback();
    sleep(SLEEP_AFTER_STARTING_PLAYBACK);
    startPreview();
    startRecording();

    pthread_mutex_lock(&mMutex);
    if (bPlaying && bRecording && !mMediaPlayerThrewError){
        my_pthread_cond_timedwait(&mCond, &mMutex, mPlaybackDuration);
    }
    pthread_mutex_unlock(&mMutex);

    stopRecording();
    stopPreview();
    stopPlayback();

    cameraWinX = 0;
    cameraWinY = 0;
    playerWinX = 0;
    playerWinY = 0;
    return 0;
}


int test_PlaybackAndRecord_PIP() {
    playbackComposerClient = new SurfaceComposerClient();
    CHECK_EQ(playbackComposerClient->initCheck(), (status_t)OK);

    uint32_t panelwidth = playbackComposerClient->getDisplayWidth(0);
    uint32_t panelheight = playbackComposerClient->getDisplayHeight(0);
    VTC_LOGD("Panel WxH = %d x %d", panelwidth, panelheight);
    if (panelwidth < panelheight) {//Portrait Phone
        VTC_LOGD("\nPortrait Device\n");
        playbackSurfaceWidth = panelwidth;
        playbackSurfaceHeight = panelheight;
        playerWinX = 0;
        playerWinY = 0;

        cameraSurfaceWidth = panelwidth/2;
        cameraSurfaceHeight = panelheight/4;
        cameraWinX = (panelwidth - cameraSurfaceWidth) / 2;
        cameraWinY = 0;
    } else { // Landscape
        VTC_LOGD("\n Landscape Device\n");
        playbackSurfaceWidth = panelwidth;
        playbackSurfaceHeight = panelheight;
        playerWinX = 0;
        playerWinY = 0;

        cameraSurfaceWidth = panelwidth/3;
        cameraSurfaceHeight = panelheight/3;
        cameraWinX = ((panelwidth - cameraSurfaceWidth) / 2) + panelwidth/4;
        cameraWinY = 0;

    }

    startPlayback();
    sleep(SLEEP_AFTER_STARTING_PLAYBACK);
    startPreview();
    startRecording();

    while (bPlaying && bRecording && !mMediaPlayerThrewError) {
        int rc;
        pthread_mutex_lock(&mMutex);
        rc = my_pthread_cond_timedwait(&mCond, &mMutex, 100);
        pthread_mutex_unlock(&mMutex);

        if (rc != ETIMEDOUT){
            break; //exit while loop
        }
        /* Move preview */
        cameraWinY +=2;
        if ((cameraWinY+ cameraSurfaceHeight) > panelheight) cameraWinY = 0;
        client->openGlobalTransaction();
        surfaceControl->setPosition(cameraWinX, cameraWinY);
        client->closeGlobalTransaction();

        if (cameraWinY >  cameraSurfaceHeight/2){
            client->openGlobalTransaction();
            surfaceControl->hide();
            client->closeGlobalTransaction();
        } else {
            client->openGlobalTransaction();
            surfaceControl->show();
            client->closeGlobalTransaction();
        }
    };

    stopRecording();
    stopPreview();
    stopPlayback();

    cameraWinX = 0;
    cameraWinY = 0;
    playerWinX = 0;
    playerWinY = 0;
    return 0;
}

int test_PlaybackOnly()
{
    playbackComposerClient = new SurfaceComposerClient();
    CHECK_EQ(playbackComposerClient->initCheck(), (status_t)OK);

    int panelwidth = playbackComposerClient->getDisplayWidth(0);
    int panelheight = playbackComposerClient->getDisplayHeight(0);
    VTC_LOGD("Panel WxH = %d x %d", panelwidth, panelheight);
    if (panelwidth < panelheight) {//Portrait Phone
        VTC_LOGD("\nPortrait Device\n");
        playbackSurfaceWidth = panelwidth;
        playbackSurfaceHeight = panelheight/2;
        playerWinX = 0;
        playerWinY = 0;

        cameraWinX = 0;
        cameraWinY = playbackSurfaceHeight;
        cameraSurfaceWidth = panelwidth;
        cameraSurfaceHeight = panelheight/2;
    } else {// Landscape
        VTC_LOGD("\n Landscape Device\n");
        playbackSurfaceWidth = panelwidth;
        playbackSurfaceHeight = panelheight;
        playerWinX = 0;
        playerWinY = 0;
    }

    startPlayback();
    pthread_mutex_lock(&mMutex);
    if (bPlaying){
        my_pthread_cond_timedwait(&mCond, &mMutex, mPlaybackDuration);
    }
    pthread_mutex_unlock(&mMutex);
    stopPlayback();
    playerWinX = 0;
    playerWinY = 0;
    return 0;

}

void updatePassRate(int test_status, bool verifyRecordedClip) {

    mTestCount++;

    if (verifyRecordedClip){
        sleep(2);
        strcpy(mPlaybackFileName,  mRecordFileName);
        verfiyByPlayback();
    }

    // Wait for 10 seconds to make sure the memory settle.
    VTC_LOGD("%d: %s: Evaluating test results. Looking for memory leak. Waiting for memory to settle down ....", __LINE__, __FUNCTION__);
    sleep(10);

    int currentMediaServerPID;
    int endMemory;
    getMediaserverInfo(&currentMediaServerPID, &endMemory);
    int memDiff = endMemory - mStartMemory;
    if (memDiff < 0) {
        memDiff = 0;
    }
    VTC_LOGD("\n\n======================= Memory Leak [in bytes] = %d =======================\n\n", memDiff);sleep(1);

    int old_mFailCount = mFailCount;
    if (mMediaPlayerThrewError) mFailCount++;
    else if (mCameraThrewError) mFailCount++;
    else if (bPlaying == false) mFailCount++;
    else if (bRecording == false) mFailCount++;
    else if (test_status != 0) mFailCount++;
    else if (mMediaServerPID != currentMediaServerPID) mFailCount++; //implies mediaserver crashed. So, increment failure count.
    else if (memDiff > 10000) mFailCount++; //implies memory leak. So, increment failure count.

    VTC_LOGD("\n\nTest Results:\n\nNo. of Tests Executed = %d\nPASS = %d\nFAIL = %d\n\n", mTestCount, (mTestCount-mFailCount), (mFailCount*-1));

    mResultsFP = fopen("/sdcard/VTC_TEST_RESULTS.TXT", "a");
    if (mResultsFP != NULL) {
        if (old_mFailCount != mFailCount)
            fprintf(mResultsFP,  "%03d\tFAIL\t[%s]\n", mTestCount, mRecordFileName);
        else fprintf(mResultsFP,  "%03d\tPASS\t[%s]\n", mTestCount, mRecordFileName);
        fclose(mResultsFP);
    }

    mMediaServerPID = currentMediaServerPID; //Initialize for next test
    mStartMemory = endMemory; //I shouldn't be doing this right??
}

int test_Robust() {
    int status = 0;
    uint32_t cyclesCompleted = 0;
    getMediaserverInfo(&mMediaServerPID, &mStartMemory);

    if (mRobustnessTestType != -1){
        if ((mRobustnessTestType >= 1) || (mRobustnessTestType <= 8)) {
            for ( cyclesCompleted = 0; cyclesCompleted < mCycles; cyclesCompleted++){
                VTC_LOGD("\n\n\n############################ Iteration: %d. Goal: %d ############################\n\n\n", cyclesCompleted, mCycles);
                status = TestFunctions[mRobustnessTestType]();
                updatePassRate(status, false);
            }
        }
        return 0;
    }

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_0039_Robustness_last_recorded.3gp");
    VTC_LOGD("\n\n################################## Recording. Filename: %s\n\n", mRecordFileName);

    // Each cycle will play a selected number of different resolution scenarios
    // Starting from low to high resolution
    for ( cyclesCompleted = 0; cyclesCompleted < mCycles; cyclesCompleted++){
        VTC_LOGD("\n\n\n############################ Iteration: %d. Goal: %d ############################\n\n\n", cyclesCompleted, mCycles);
        mVideoBitRate = 3000000;
        mVideoFrameRate = 30;

        sprintf(mPlaybackFileName,  "/mnt/ext_sdcard/vtc_playback/AV_000249_H264_VGA_1Mbps_eAACplus_48khz_64kbps.mp4");
        mPreviewWidth = 640;
        mPreviewHeight = 480;
        status = test_PlaybackAndRecord_sidebyside();
        updatePassRate(status, false);

        sprintf(mPlaybackFileName,  "/mnt/ext_sdcard/vtc_playback/AV-720p-JamesBond.MP4");
        mPreviewWidth = 1280;
        mPreviewHeight = 720;
        status = test_PlaybackAndRecord_sidebyside();
        updatePassRate(status, false);
    }

    return 0;
}


int test_ALL()
{
    // Automated Unit Test suite
    VTC_LOGD("\n\nExecuting %s \n\n", __FUNCTION__);
    int status = 0;
    char value[PROPERTY_VALUE_MAX];
    getMediaserverInfo(&mMediaServerPID, &mStartMemory);
    mDuration = 10;
    mResultsFP = fopen("/sdcard/VTC_TEST_RESULTS.TXT", "w+"); // To create a new file each time
    fclose(mResultsFP);

    property_get("disable.1080p.testing", value, "0");
    mDisable1080pTesting = atoi(value);
    if(mDisable1080pTesting){
        VTC_LOGD("\n\n\n\n########  1080p Testing as been disable  #######\n\n\n");
    }

  if(!mDisable1080pTesting){
    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_30fps_1Mbps_i-frame-2sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mIFramesIntervalSec = 2;
    status = test_DEFAULT();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_30fps_1Mbps_i-frame-10sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mIFramesIntervalSec = 10;
    mDuration = 30;
    status = test_DEFAULT();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_30fps_1Mbps_i-frame-0sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mIFramesIntervalSec = 0;
    status = test_DEFAULT();
    updatePassRate(status, true);
    }

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_30fps_1Mbps_i-frame-3sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mIFramesIntervalSec = 3;
    status = test_DEFAULT();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_30fps_1Mbps_i-frame-8sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mIFramesIntervalSec = 8;
    status = test_DEFAULT();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_30fps_1Mbps_i-frame-0sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mIFramesIntervalSec = 0;
    status = test_DEFAULT();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGA_30fps_1Mbps_i-frame-1sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    mIFramesIntervalSec = 1;
    status = test_DEFAULT();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGA_30fps_1Mbps_i-frame-15sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    mIFramesIntervalSec = 15;
    mDuration = 30;
    status = test_DEFAULT();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGA_30fps_1Mbps_i-frame-0sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    mIFramesIntervalSec = 0;
    status = test_DEFAULT();
    updatePassRate(status, true);

  if(!mDisable1080pTesting){
    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_1Mbps_30fps-15fps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mVideoFrameRate = 30;
    mNewVideoFrameRate = 15;
    status = test_ChangeFrameRate();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_1Mbps_15fps-30fps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mVideoFrameRate = 15;
    mNewVideoFrameRate = 30;
    status = test_ChangeFrameRate();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_1Mbps_30fps-24fps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mVideoFrameRate = 30;
    mNewVideoFrameRate = 24;
    status = test_ChangeFrameRate();
    updatePassRate(status, true);
    }

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_1Mbps_30fps-15fps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mVideoFrameRate = 30;
    mNewVideoFrameRate = 15;
    status = test_ChangeFrameRate();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_1Mbps_15fps-30fps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mVideoFrameRate = 30;
    mNewVideoFrameRate = 24;
    status = test_ChangeFrameRate();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_1Mbps_24fps-30fps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mVideoFrameRate = 15;
    mNewVideoFrameRate = 30;
    status = test_ChangeFrameRate();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGA_1Mbps_30fps-15fps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    mVideoFrameRate = 30;
    mNewVideoFrameRate = 15;
    status = test_ChangeFrameRate();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGA_1Mbps_15fps-30fps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    mVideoFrameRate = 30;
    mNewVideoFrameRate = 24;
    status = test_ChangeFrameRate();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGA_1Mbps_24fps-30fps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    mVideoFrameRate = 24;
    mNewVideoFrameRate = 30;
    status = test_ChangeFrameRate();
    updatePassRate(status, true);

  if(!mDisable1080pTesting){
    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_30fps_1Mbps_max-1000MB.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mVideoFrameRate = 30;
    mIsSizeInBytes = false;
    mSliceSizeMB = 1000;
    status = test_MaxNALSize();
    updatePassRate(status, true);
    }

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_30fps_1Mbps_max-300MB.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mIsSizeInBytes = false;
    mSliceSizeMB = 300;
    status = test_MaxNALSize();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGAp_30fps_1Mbps_max-8MB.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    mIsSizeInBytes = false;
    mSliceSizeMB = 8;
    status = test_MaxNALSize();
    updatePassRate(status, true);

  if(!mDisable1080pTesting){
    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_30fps_1Mbps_i-frames_every-2sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    InsertIDRFrameEveryXSecs = 2;
    status = test_InsertIDRFrames();
    updatePassRate(status, true);
    }

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_30fps_1Mbps_i-frames_every-5sec.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    InsertIDRFrameEveryXSecs = 5;
    status = test_InsertIDRFrames();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGA_30fps_1Mbps_i-frames_every-8.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    InsertIDRFrameEveryXSecs = 8;
    mDuration = 20;
    status = test_InsertIDRFrames();
    updatePassRate(status, true);

    if(!mDisable1080pTesting){
    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_30fps_1Mbps_max-1000bytes.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mIsSizeInBytes = true;
    mSliceSizeBytes = 1000;
    mVideoBitRate = 1000000;
    mVideoFrameRate = 30;
    status = test_MaxNALSize();
    updatePassRate(status, true);
    }

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_30fps_1Mbps_max-500bytes.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mIsSizeInBytes = true;
    mSliceSizeBytes = 500;
    mVideoBitRate = 1000000;
    mVideoFrameRate = 30;
    status = test_MaxNALSize();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGA_30fps_1Mbps_max-256bytes.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    mIsSizeInBytes = true;
    mSliceSizeBytes = 256;
    mVideoBitRate = 1000000;
    mVideoFrameRate = 30;
    status = test_MaxNALSize();
    updatePassRate(status, true);

    //PIP TC
    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_D1PAL_30fps_1Mbps_simultaneous.3gp", mTestCount);
    sprintf(mPlaybackFileName,  "/mnt/ext_sdcard/vtc_playback/AV_000795_H264_D1PAL_25fps_4Mbps_NB_AMR_8Khz_12.2Kbps.mp4");
    VTC_LOGD("\n\n################################## Recording. Filename: %s\n\n", mRecordFileName);
    VTC_LOGD("\n\n################################## Playing. Filename: %s\n\n", mPlaybackFileName);
    mPreviewWidth = 720;
    mPreviewHeight = 576;
    mVideoBitRate = 1000000;
    mVideoFrameRate = 30;
    status = test_PlaybackAndRecord_sidebyside();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_D1PAL_30fps_1Mbps_PIP.3gp", mTestCount);
    sprintf(mPlaybackFileName,  "/mnt/ext_sdcard/vtc_playback/AV_000795_H264_D1PAL_25fps_4Mbps_NB_AMR_8Khz_12.2Kbps.mp4");
    VTC_LOGD("\n\n################################## Recording. Filename: %s\n\n", mRecordFileName);
    VTC_LOGD("\n\n################################## Playing. Filename: %s\n\n", mPlaybackFileName);
    mPreviewWidth = 720;
    mPreviewHeight = 576;
    mVideoBitRate = 1000000;
    mVideoFrameRate = 30;
    status = test_PlaybackAndRecord_PIP();
    updatePassRate(status, true);

    //Simultaneous playback/record
    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGA_30fps_1Mbps_simultaneous.3gp", mTestCount);
    sprintf(mPlaybackFileName,  "/mnt/ext_sdcard/vtc_playback/AV_000249_H264_VGA_1Mbps_eAACplus_48khz_64kbps.mp4");
    VTC_LOGD("\n\n################################## Recording. Filename: %s\n\n", mRecordFileName);
    VTC_LOGD("\n\n################################## Playing. Filename: %s\n\n", mPlaybackFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    mVideoBitRate = 1000000;
    mVideoFrameRate = 30;
    status = test_PlaybackAndRecord_sidebyside();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_24fps_1Mbps_simultaneous.3gp", mTestCount);
    sprintf(mPlaybackFileName,  "/mnt/ext_sdcard/vtc_playback/FinalFantasy13_720p_mono_3.8Mbps_27fps.MP4");
    VTC_LOGD("\n\n################################## Recording. Filename: %s\n\n", mRecordFileName);
    VTC_LOGD("\n\n################################## Playing. Filename: %s\n\n", mPlaybackFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mVideoBitRate = 1000000;
    mVideoFrameRate = 24;
    status = test_PlaybackAndRecord_sidebyside();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_30fps_1Mbps_simultaneous.3gp", mTestCount);
    sprintf(mPlaybackFileName,  "/mnt/ext_sdcard/vtc_playback/AV-720p-JamesBond.MP4");
    VTC_LOGD("\n\n################################## Recording. Filename: %s\n\n", mRecordFileName);
    VTC_LOGD("\n\n################################## Playing. Filename: %s\n\n", mPlaybackFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mVideoBitRate = 1000000;
    mVideoFrameRate = 30;
    status = test_PlaybackAndRecord_sidebyside();
    updatePassRate(status, true);

    if(!mDisable1080pTesting){
    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_24fps_1Mbps_simultaneous.3gp", mTestCount);
    sprintf(mPlaybackFileName,  "/mnt/ext_sdcard/vtc_playback/AV_001181_Toy_Story3Official_Trailer_in_FullHD1080p_h264_BP_L4.0_1920x1080_24fps_1Mbps_eAACplus_44100Hz.mp4");
    VTC_LOGD("\n\n################################## Recording. Filename: %s\n\n", mRecordFileName);
    VTC_LOGD("\n\n################################## Playing. Filename: %s\n\n", mPlaybackFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mVideoBitRate = 1000000;
    mVideoFrameRate = 24;
    status = test_PlaybackAndRecord_sidebyside();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_1080p_30fps_1Mbps_simultaneous.3gp", mTestCount);
    sprintf(mPlaybackFileName,  "/mnt/ext_sdcard/vtc_playback/AV_000858_FinalFantasy13_1080p_h264_bp_30fps_8mbps_aac_lc.mp4");
    VTC_LOGD("\n\n################################## Recording. Filename: %s\n\n", mRecordFileName);
    VTC_LOGD("\n\n################################## Playing. Filename: %s\n\n", mPlaybackFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mVideoBitRate = 1000000;
    mVideoFrameRate = 30;
    status = test_PlaybackAndRecord_sidebyside();
    updatePassRate(status, true);
    }

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_QQVGA_15fps_bps_i-frames_every-8.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 160;
    mPreviewHeight = 120;
    mDuration = 10;
    mVideoBitRate = 262144;
    mVideoFrameRate = 15;
    status = test_DEFAULT();
    updatePassRate(status, true);
    //Framerate and Bitrate change TC done, set default
    mVideoFrameRate = 30;
    mVideoBitRate = 1000000;

  if(!mDisable1080pTesting){
    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d1_1080p_30fps_from-5Mbps-to-100kbps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1920;
    mPreviewHeight = 1080;
    mNewVideoBitRate = 100000;
    mVideoBitRate = 5000000;
    status = test_ChangeBitRate();
    updatePassRate(status, true);
    }

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_720p_30fps_from-5Mbps-to-100kbps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 1280;
    mPreviewHeight = 720;
    mNewVideoBitRate = 100000;
    mVideoBitRate = 5000000;
    status = test_ChangeBitRate();
    updatePassRate(status, true);

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/UTR_%03d_VGA_30fps_from-5Mbps-to-100kbps.3gp", mTestCount);
    VTC_LOGD("\n\n###################################################### Recording. Filename: %s\n\n", mRecordFileName);
    mPreviewWidth = 640;
    mPreviewHeight = 480;
    mNewVideoBitRate = 50000;
    mVideoBitRate = 1000000;
    status = test_ChangeBitRate();
    updatePassRate(status, true);

    //Bit rate change TC done, set default
    mVideoBitRate = 1000000;

    /////////////////////////////     END   of  Unit Test     /////////////////////////////////////////

    VTC_LOGD("\n\nTest Results:\n\nNo. of Tests = %d\nPASS = %d\nFAIL = %d\n\n", mTestCount, (mTestCount-mFailCount), (mFailCount*-1));
    char results[256];
    mResultsFP = fopen("/sdcard/VTC_TEST_RESULTS.TXT", "r");
    if (mResultsFP != NULL) {
        while( fgets(results, sizeof(results), mResultsFP) != NULL ) VTC_LOGD("%s", results);
        fclose(mResultsFP);
    }
    return 0;
}


void printUsage() {
    printf("\n\nApplication for testing VTC requirements");
    printf("\nUsage: /system/bin/VTCTestApp test_case_id");
    printf("\n\n\nTest Case ID can be any of the following");
    printf("\n0 - Runs all the tests listed in the UTR. Pass/Fail must be determined by the tester after examining the recorded clips.");
    printf("\n1 - Default test case. Does not require any parameters to be set.");
    printf("\n2 - Tests the ability to request I-Frame generation real time. Option: -I");
    printf("\n3 - Tests the ability to specify the maximum allowed frame size. Option: -s");
    printf("\n4 - Tests the ability to change the bitrate at runtime. Option: -B");
    printf("\n5 - Tests the ability to change the framerate at runtime. Option: -F");
    printf("\n6 - Test Playback and Record. Option: -p, -n");
    printf("\n7 - Test PIP. Option: -p, -n");
    printf("\n8 - Test Video playback Only. Option: -p");
    printf("\n9 - Robustness. Default: play and record the predefined resolutions (VGA & 720p). Option: -c, -v");

    printf("\n\n\nAvailable Options:");
    printf("\n-n: Record Filename(/mnt/sdcard/vtc_videos/video_0.3gp) is appended with this number. Default = %d", filename);
    printf("\n-w: Preview/Record Width. Default = %d", mPreviewWidth);
    printf("\n-h: Preview/Record Height. Default = %d", mPreviewHeight);
    printf("\n-d: Recording time in secs. Default = %d seconds", mDuration);
    printf("\n-i: I Frame Interval in secs. Default = %d", mIFramesIntervalSec);
    printf("\n-s: Slice Size bytes. Default = %d", mSliceSizeBytes);
    printf("\n-M: Slice Size Macroblocks. Default = %d", mSliceSizeMB);
    printf("\n-b: Bitrate. Default = %d", mVideoBitRate);
    printf("\n-f: Framerate. Default = %d", mVideoFrameRate);
    printf("\n-I: Insert I Frames. Specify the period in secs. Default = %d second", InsertIDRFrameEveryXSecs);
    printf("\n-B: Change bitrate at runtime to this new value. Default = %d", mNewVideoBitRate);
    printf("\n-F: Change framerate at runtime to this new value. Default = %d", mNewVideoFrameRate);
    printf("\n-p: Playback Filename. Default = %s", mPlaybackFileName);
    printf("\n-c: Robustness, number of cycles. Default = %d", mCycles);
    printf("\n-v: Robustness. Which test to repeat? Range: 1-8. Read test descriptions above. Default = -1 = play and record the predefined resolutions (VGA & 720p)");
    printf("\n-t: Playback Duration in milli secs. Default = %d = Play till EOF", mPlaybackDuration);
    printf("\n\n\n");

}

int main (int argc, char* argv[]) {
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();
    pthread_mutex_init(&mMutex, NULL);
    pthread_cond_init(&mCond, NULL);

    int opt;
    const char* const short_options = "n:w:h:d:s:i:I:b:f:B:F:p:M:c:t:v:";
    const struct option long_options[] = {
        {"record_filename", 1, NULL, 'n'},
        {"width", 1, NULL, 'w'},
        {"height", 1, NULL, 'h'},
        {"record_duration", 1, NULL, 'd'},
        {"slice_size_bytes", 1, NULL, 's'},
        {"i_frame_interval", 1, NULL, 'i'},
        {"insert_I_frames", 1, NULL, 'I'},
        {"bitrate", 1, NULL, 'b'},
        {"new_bitrate", 1, NULL, 'B'},
        {"framerate", 1, NULL, 'f'},
        {"new_framerate", 1, NULL, 'F'},
        {"playback_filename", 1, NULL, 'p'},
        {"slice_size_MB", 1, NULL, 'M'},
        {"num_cycles", 1, NULL, 'c'},
        {"playback_duration", 1, NULL, 't'},
        {"robustness_test_type", 1, NULL, 'v'},
        {NULL, 0, NULL, 0}
    };

    sprintf(mPlaybackFileName,  "/sdcard/nasa.mp4");

    if (argc < 2){
        printUsage();
        return 0;
    }
    testcase = atoi(argv[1]);

    while((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch(opt) {
            case 'n':
                filename = atoi(optarg);
                break;
            case 'w':
                mPreviewWidth = atoi(optarg);
                break;
            case 'h':
                mPreviewHeight = atoi(optarg);
                break;
            case 'd':
                mDuration = atoi(optarg);
                break;
            case 's':
                mSliceSizeBytes = atoi(optarg);
                mIsSizeInBytes = true;
                break;
            case 'b':
                mVideoBitRate = atoi(optarg);
                break;
            case 'B':
                mNewVideoBitRate = atoi(optarg);
                break;
            case 'f':
                mVideoFrameRate = atoi(optarg);
                break;
            case 'F':
                mNewVideoFrameRate = atoi(optarg);
                break;
            case 'i':
                mIFramesIntervalSec = atoi(optarg);
                break;
            case 'I':
                InsertIDRFrameEveryXSecs = atoi(optarg);
                break;
            case 'p':
                strcpy(mPlaybackFileName, optarg);
                VTC_LOGD("Playback clip %s", mPlaybackFileName);
                break;
            case 'M':
                mSliceSizeMB = atoi(optarg);
                mIsSizeInBytes = false;
                break;
            case 'c':
                mCycles = atoi(optarg);
                break;
            case 't':
                mPlaybackDuration = atoi(optarg);
                break;
            case 'v':
                mRobustnessTestType = atoi(optarg);
                break;
            case ':':
                VTC_LOGE("\nError - Option `%c' needs a value\n\n", optopt);
                return -1;
            case '?':
                VTC_LOGE("\nError - No such option: `%c'\n\n", optopt);
                return -1;
        }
    }

    sprintf(mRecordFileName,  "/mnt/sdcard/vtc_videos/video_%d.3gp", filename);
    VTC_LOGI("\n\nRecorded Output is stored in %s\n\n", mRecordFileName);
    system("echo VTCTestApp > /sys/power/wake_lock");
    TestFunctions[testcase]();
    system("echo VTCTestApp > /sys/power/wake_unlock");
    pthread_mutex_destroy(&mMutex);
    pthread_cond_destroy(&mCond);
    return 0;
}

