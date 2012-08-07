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

#ifndef IOMX_ENCODER_H
#define IOMX_ENCODER_H

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

#include "VtcCommon.h"


using namespace android;

typedef void (*EncodedBufferCallback)(void* pBuffer, OMX_U32 nFilledLen, OMX_TICKS nTimeStamp);

struct OMXEncoderObserver;

struct OMXEncoder : public MediaSource {
    struct BufferInfo {
        OMX_BUFFERHEADERTYPE* mBufferHdr;
        sp<IMemory> mEncMem;
        sp<IMemory> mCamMem;
    };

    friend class OMXEncoderObserver;

    status_t resetParameters(int width, int height, int framerate, int bitrate, char *fname, int sliceHeight);
    status_t configure(OMX_VIDEO_AVCPROFILETYPE profile, OMX_VIDEO_AVCLEVELTYPE level, OMX_U32 refFrames);
    status_t prepare();
    status_t start(MetaData *params = NULL);
    status_t stop();
    status_t deinit();
    status_t changeFrameRate(int framerate);
    status_t changeBitRate(int bitrate);
    status_t setEncoderOutputSlice(OMX_U32 nHeight, OMX_U32 nWidth, OMX_U32 sizeBytes, OMX_U32 sizeMB);
    sp<MetaData> getFormat();
    status_t read(MediaBuffer **buffer, const ReadOptions *options = NULL);
    void on_message(const omx_message &msg);
    void setCallback(EncodedBufferCallback fp);
    OMXEncoder(const sp<IOMX> &omx, IOMX::node_id node, sp<MyCameraClient> camera, int width, int height, int framerate, int bitrate, char *fname, int sliceHeight);
    OMXEncoder(const OMXEncoder &);
    OMXEncoder &operator=(const OMXEncoder &);
    ~OMXEncoder();
    uint32_t mDebugFlags;
    uint32_t mOutputBufferCount;

private:

    class OMXCallbackHandler : public Thread {
    public:
        OMXCallbackHandler(OMXEncoder* enc)
            : Thread(false), mOMXEncoder(enc) { }

        virtual bool threadLoop() {
            bool ret;
            ret = Handler();
            return ret;
        }

        status_t put(Ti::Utils::Message* msg) {
            Mutex::Autolock lock(mLock);
            return mCommandMsgQ.put(msg);
        }

        void clearCommandQ() {
            Mutex::Autolock lock(mLock);
            mCommandMsgQ.clear();
        }

        enum {
            COMMAND_EXIT = -1,
            COMMAND_PROCESS_MSG,
        };

    private:
        bool Handler();
        Ti::Utils::MessageQueue mCommandMsgQ;
        OMXEncoder* mOMXEncoder;
        Mutex mLock;
    };

    sp<OMXCallbackHandler> mOMXCallbackHandler;

    OMX_ERRORTYPE EventHandler(OMX_EVENTTYPE eEvent, OMX_U32 nData1,OMX_U32 nData2);
    status_t FillBufferDone(OMX_BUFFERHEADERTYPE* pBufferHdr, OMX_U32 nOffset, OMX_U32 nFilledLen, OMX_TICKS nTimeStamp);
    status_t EmptyBufferDone(OMX_BUFFERHEADERTYPE* pBufferHdr);
    status_t setCurrentState(OMX_STATETYPE newState);
    status_t waitForStateSet(OMX_STATETYPE newState);

    int mWidth;
    int mHeight;
    uint32_t mBitRate;
    uint32_t mFrameRate;
    sp<MyCameraClient> mCameraSource;
    OMX_PARAM_PORTDEFINITIONTYPE tInPortDef;
    OMX_PARAM_PORTDEFINITIONTYPE tOutPortDef;
    sp<IOMX> mOMX;
    IOMX::node_id mNode;
    BufferInfo mBufferInfo[NUM_PORTS][ENCODER_MAX_BUFFER_COUNT];
    Condition mAsyncCompletion;
    Mutex mLock;
    OMX_STATETYPE mState;
    int mAcceptingBuffers;
    sp<MemoryDealer> mDealer[NUM_PORTS];
    FILE* mOutputFD;
    int mSliceHeight;
    int mBufferCount;
    EncodedBufferCallback mEncodedBufferCallback;
    bool mCallbackSet;
};

struct OMXEncoderObserver : public BnOMXObserver {
    OMXEncoderObserver() {
    }

    void setCodec(const sp<OMXEncoder> &target) {
        mTarget = target;
    }

    // from IOMXObserver
    virtual void onMessage(const omx_message &omx_msg) {
        Ti::Utils::Message msg;
        omx_message *ptemp_omx_msg;
        // TODO: Check on the memory scope of below allocation
        ptemp_omx_msg = (omx_message *)malloc(sizeof(omx_message));
        memcpy(ptemp_omx_msg, &omx_msg, sizeof(omx_message));
        //LOGD("=================omx_msg.type = %x, temp_omx_msg.type = %x",omx_msg.type, ptemp_omx_msg->type);
        sp<OMXEncoder> codec = mTarget.promote();
        if (codec.get() != NULL) {
            msg.command = OMXEncoder::OMXCallbackHandler::COMMAND_PROCESS_MSG;
            msg.arg1 = (void *)ptemp_omx_msg;
            codec->mOMXCallbackHandler->put(&msg);
            codec.clear();
        }
    }

protected:
    virtual ~OMXEncoderObserver() {}

private:
    wp<OMXEncoder> mTarget;
    OMXEncoderObserver(const OMXEncoderObserver &);
    OMXEncoderObserver &operator=(const OMXEncoderObserver &);
};

#endif
