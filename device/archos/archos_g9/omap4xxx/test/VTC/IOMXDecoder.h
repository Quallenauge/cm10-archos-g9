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

#ifndef IOMX_DECODER_H
#define IOMX_DECODER_H

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
#include <media/stagefright/OMXClient.h>
#include <media/stagefright/MediaDefs.h>
#ifdef ANDROID_API_JB_OR_LATER
#include <media/stagefright/foundation/ADebug.h>
#else
#include <media/stagefright/MediaDebug.h>
#endif

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
#include <android/native_window.h>
#include <ui/GraphicBuffer.h>

#include "MessageQueue.h"

#include "VtcCommon.h"


using namespace android;

struct OMXDecoderObserver;

struct OMXDecoder : public MediaSource
{
    enum BufferStatus {
        OWNED_BY_US,
        OWNED_BY_COMPONENT,
        OWNED_BY_NATIVE_WINDOW,
        OWNED_BY_CLIENT,
    };

    struct BufferInfo {
        void* pBuffer;
        OMX_U32 nFilledLen;
        OMX_TICKS nTimeStamp;
    };

    struct PortBufferInfo{
        sp<GraphicBuffer> gb;
        IOMX::buffer_id b_id;
        BufferStatus mStatus;
    };

    struct InPortBufferInfo{
        sp<IMemory> mem;
        IOMX::buffer_id b_id;
        OMX_U32 nFilledLen;
        OMX_TICKS nTimeStamp;
    };

    friend class OMXDecoderObserver;

    status_t configure(OMX_VIDEO_AVCPROFILETYPE profile, OMX_VIDEO_AVCLEVELTYPE level, OMX_U32 refFrames);
    status_t prepare();
    status_t start(MetaData *params = NULL);
    status_t stop();
    sp<MetaData> getFormat();
    status_t read(MediaBuffer **buffer, const ReadOptions *options = NULL);
    void on_message(const omx_message &msg);
    void AcceptEncodedBuffer(void *pBuffer, OMX_U32 nFilledLen, OMX_TICKS nTimeStamp);
    OMXDecoder(int width, int height, int framerate);
    OMXDecoder(const OMXDecoder &);
    OMXDecoder &operator=(const OMXDecoder &);
    ~OMXDecoder();
    uint32_t mDebugFlags;

private:

    class SourceHandler : public Thread {
    public:
        SourceHandler(OMXDecoder* dec)
            : Thread(false), mOMXDecoder(dec) { }

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
        OMXDecoder* mOMXDecoder;
        Mutex mLock;
    };

    sp<SourceHandler> mSourceHandler;

    class OMXCallbackHandler : public Thread {
    public:
        OMXCallbackHandler(OMXDecoder* dec)
            : Thread(false), mOMXDecoder(dec) { }

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
        OMXDecoder* mOMXDecoder;
        Mutex mLock;
    };

    sp<OMXCallbackHandler> mOMXCallbackHandler;

    OMX_ERRORTYPE EventHandler(OMX_EVENTTYPE eEvent, OMX_U32 nData1,OMX_U32 nData2);
    status_t FillBufferDone(OMX_BUFFERHEADERTYPE* pBufferHdr);
    status_t EmptyBufferDone(OMX_BUFFERHEADERTYPE* pBufferHdr);
    status_t setCurrentState(OMX_STATETYPE newState);
    status_t waitForStateSet(OMX_STATETYPE newState);
    status_t createPlaybackSurface();
    status_t destroyPlaybackSurface();
    status_t allocateOutputBuffersFromNativeWindow();
    status_t drainInputBuffer(InPortBufferInfo *info);
    status_t freeOutputBuffers();
    status_t restart();

    int mWidth;
    int mHeight;
    int mSizeOfAllAllocatedOutputBuffers;
    uint32_t mFrameRate;
    OMX_PARAM_PORTDEFINITIONTYPE tInPortDef;
    OMX_PARAM_PORTDEFINITIONTYPE tOutPortDef;
    sp<IOMX> mOMX;
    IOMX::node_id mNode;
    sp<OMXDecoderObserver> mObserver;
    OMXClient mOMXClient;
    Vector<PortBufferInfo> mOutputBuffers;
    Vector<InPortBufferInfo*> mInputBuffers;
    List<InPortBufferInfo*> mEmptyInputBuffers;
    Condition mAsyncCompletion;
    Mutex mLock;
    int waitForStateChange;
    OMX_STATETYPE mState;
    int mAcceptingBuffers;
    sp<MemoryDealer> mDealer;
    sp<SurfaceComposerClient> mSurfaceComposerClient;
    sp<SurfaceControl> mSurfaceControl;
    sp<ANativeWindow> mNativeWindow;
    bool mPortReconfigInProgress;
};

struct OMXDecoderObserver : public BnOMXObserver {
    OMXDecoderObserver() {
    }

    void setCodec(const sp<OMXDecoder> &target) {
        mTarget = target;
    }

    // from IOMXObserver
    virtual void onMessage(const omx_message &omx_msg) {
        Ti::Utils::Message msg;
        omx_message *ptemp_omx_msg;
        // HACK HACK HACK LEAK LEAK LEAK FIXIT
        ptemp_omx_msg = (omx_message *)malloc(sizeof(omx_message));
        memcpy(ptemp_omx_msg, &omx_msg, sizeof(omx_message));
        //LOGD("=================omx_msg.type = %x, temp_omx_msg.type = %x",omx_msg.type, ptemp_omx_msg->type);
        sp<OMXDecoder> codec = mTarget.promote();
        if (codec.get() != NULL) {
            msg.command = OMXDecoder::OMXCallbackHandler::COMMAND_PROCESS_MSG;
            msg.arg1 = (void *)ptemp_omx_msg;
            codec->mOMXCallbackHandler->put(&msg);
            codec.clear();
        }
    }

protected:
    virtual ~OMXDecoderObserver() {}

private:
    wp<OMXDecoder> mTarget;
    OMXDecoderObserver(const OMXDecoderObserver &);
    OMXDecoderObserver &operator=(const OMXDecoderObserver &);
};

#endif

