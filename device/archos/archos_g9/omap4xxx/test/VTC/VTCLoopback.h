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

#ifndef VTC_LOOPBACK_H
#define VTC_LOOPBACK_H

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

#include <binder/MemoryDealer.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <system/audio.h>
#include <utils/List.h>
#include <cutils/log.h>
#include <OMX_Component.h>
#include <camera/Camera.h>
#include <camera/ICamera.h>
#include <camera/ICameraClient.h>
#include <camera/ICameraService.h>
#include <media/mediaplayer.h>
#include <media/mediarecorder.h>
#include <media/stagefright/OMXClient.h>
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

#include "OMX_TI_Index.h"   // for OMX_TI_VIDEO_PARAM_FRAMEDATACONTENTTYPE
#include "OMX_TI_Video.h"   // for OMX_VIDEO_PARAM_DATASYNCMODETYPE
#include "OMX_TI_Common.h"  // for OMX_TI_COMPONENT_HANDLE
#include "OMX_TI_IVCommon.h"// for OMX_TI_COLOR_FormatYUV420PackedSemiPlanar

#include "MessageQueue.h"

#include "VtcCommon.h"


#define SLEEP_AFTER_STARTING_PREVIEW 2
#define WIDTH 640
#define HEIGHT 480
#define BITRATE 5000000 // 5 mbps
#define INPUT_PORT  0
#define OUTPUT_PORT 1
#define TWO_SECOND 1000000000ll  // 1000,000,000 nsec = 1sec

#define FPS_CAMERA 0x1
#define FPS_ENCODER 0X2
#define FPS_DECODER 0x4
#define DEBUG_DUMP_CAMERA_TIMESTAMP 0x8
#define DEBUG_DUMP_ENCODER_TIMESTAMP 0x10
#define ENCODER_EFFECTIVE_BITRATE 0x20
#define ENCODER_NO_FILE_WRTIE 0x40
#define INPUT_SLICE_MODE 0x80
#define INPUT_OUTPUT_SLICE_MODE 0x100
#define ENCODER_LATENCY 0x200
#define DECODER_LATENCY 0x400

#define ENCODER_MAX_BUFFER_COUNT 10
#define NUM_PORTS 2

#define INIT_OMX_STRUCT(_s_, _name_)   \
    memset((_s_), 0x0, sizeof(_name_));         \
    (_s_)->nSize = sizeof(_name_);              \
    (_s_)->nVersion.s.nVersionMajor = 0x1;      \
    (_s_)->nVersion.s.nVersionMinor = 0x1;      \
    (_s_)->nVersion.s.nRevision = 0x0;          \
    (_s_)->nVersion.s.nStep = 0x0

using namespace android;



class MyCameraClient : public BnCameraClient {
public:
    virtual void notifyCallback(int32_t msgType, int32_t ext1, int32_t ext2) {}
    virtual void dataCallback(int32_t msgType, const sp<IMemory>& data,
            camera_frame_metadata_t *metadata){}
    virtual void dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& data);

    MyCameraClient();
    ~MyCameraClient() {}
    sp<IMemory> getCameraPayload(int64_t& frameTime);
    void encoderReady() { encoder_is_ready = 1; }
    void encoderNotReady() { encoder_is_ready = 0; }
    void releaseBuffer(sp<IMemory> data)
         { if (mReleaser != NULL) { mReleaser->releaseRecordingFrame(data); } }
    void setReleaser(ICamera *releaser) {
        mReleaser = releaser;
    }

private:

    void putCameraPayload(sp<IMemory> payload, int64_t frameTime);
    int encoder_is_ready;
    ICamera *mReleaser;
    List<sp<IMemory> > cameraPayloadQueue;
    List<int64_t> frameTimeQueue;
    Mutex cameraPayloadQueueMutex;
    Condition cameraPayloadWait;
    int cameraPayloadWaitFlag;

};

void dump_video_port_values(OMX_PARAM_PORTDEFINITIONTYPE& def);
const char *OMXStateName(OMX_STATETYPE state);


#endif

