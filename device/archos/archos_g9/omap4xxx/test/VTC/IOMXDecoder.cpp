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
#include "IOMXDecoder.h"
#define LOG_TAG "VTC_DEC"
#define LOG_NDEBUG 0

#define NO_PORT_RECONFIG        1

//Can have '7' in all cases except 1080p slice mode when VNF is enabled and
//1080p frame mode with Vstab as we will reach Tiler 2-D boundary and alloc fails
#define MAX_OUTPUT_BUF_NUM     5

#define MAX_FRAME_WIDTH        2048 //For 1080p
#define MAX_FRAME_HEIGHT       1184 //For 1080p

#define MAX_FRAME_WIDTH_720P        1408 //For 720p
#define MAX_FRAME_HEIGHT_720P       832  //For 720p

using namespace android;


static void PrintDecoderFPS() {
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
        VTC_LOGD("Decoder: %d Frames, %f FPS", mFrameCount, mFps);
    }
    // XXX: mFPS has the value we want
}

static void PrintVTCLatency(nsecs_t ts) {
    static int mFrameCount = 0;
    static int64_t sum_latency_ms = 0;
    int64_t now, diff_ms, avg_latency;

    now = systemTime()/1000;
    diff_ms = (now - ts) / 1000;
    sum_latency_ms += diff_ms;
    mFrameCount++;
    if (!(mFrameCount & 0x1F)) {
        avg_latency = sum_latency_ms / 32;
        sum_latency_ms = 0;
        VTC_LOGD("Avg Latency: %d Frames, %llu ms", mFrameCount, avg_latency);
    }
}

bool OMXDecoder::SourceHandler::Handler() {
    VTC_LOGV("\n SourceHandler::Handler \n");
    Ti::Utils::Message msg;
    volatile int forever = 1;

    while(forever) {
        Ti::Utils::MessageQueue::waitForMsg(&mCommandMsgQ, NULL, NULL, -1);
        {
            Mutex::Autolock lock(mLock);
            mCommandMsgQ.get(&msg);
        }

        switch ( msg.command ) {
            case SourceHandler::COMMAND_PROCESS_MSG: {
                InPortBufferInfo *info = (InPortBufferInfo *)(msg.arg1);
                mOMXDecoder->drainInputBuffer(info);
                break;
            }
            case SourceHandler::COMMAND_EXIT: {
                VTC_LOGD("Exiting OMX callback handler");
                forever = 0;
                break;
            }
        }
    }
    return false;
}


bool OMXDecoder::OMXCallbackHandler::Handler() {
    VTC_LOGV("\n OMXCallbackHandler::Handler \n");
    Ti::Utils::Message msg;
    volatile int forever = 1;

    while(forever) {
        Ti::Utils::MessageQueue::waitForMsg(&mCommandMsgQ, NULL, NULL, -1);
        {
            Mutex::Autolock lock(mLock);
            mCommandMsgQ.get(&msg);
        }

        switch ( msg.command ) {
            case OMXCallbackHandler::COMMAND_PROCESS_MSG: {
                omx_message *om = (omx_message*)(msg.arg1);
                omx_message omsg = *om;
                mOMXDecoder->on_message(omsg);
                break;
            }
            case OMXCallbackHandler::COMMAND_EXIT: {
                VTC_LOGD("Exiting OMX callback handler");
                forever = 0;
                break;
            }
        }
    }
    return false;
}


OMXDecoder::OMXDecoder(int width, int height, int framerate):
        mWidth(width),
        mHeight(height),
        mFrameRate(framerate),
        mAcceptingBuffers(0),
        mPortReconfigInProgress(false),
        mSizeOfAllAllocatedOutputBuffers(0) {

}

OMXDecoder::~OMXDecoder() {

}

void OMXDecoder::on_message(const omx_message &msg) {
    switch (msg.type) {
        case omx_message::EVENT:
            EventHandler(msg.u.event_data.event, msg.u.event_data.data1, msg.u.event_data.data2);
            break;
        case omx_message::EMPTY_BUFFER_DONE:
            EmptyBufferDone((OMX_BUFFERHEADERTYPE*)msg.u.extended_buffer_data.buffer);
            break;
        case omx_message::FILL_BUFFER_DONE:
            PrintVTCLatency(msg.u.extended_buffer_data.timestamp);
            FillBufferDone((OMX_BUFFERHEADERTYPE*)msg.u.extended_buffer_data.buffer);
            break;
        default:
            CHECK(!"############ Corrupted Message !!! #############");
            break;
    }
}

status_t OMXDecoder::configure(OMX_VIDEO_AVCPROFILETYPE profile, OMX_VIDEO_AVCLEVELTYPE level, OMX_U32 refFrames) {
    status_t err;
    LOG_FUNCTION_NAME_ENTRY

    createPlaybackSurface();

    CHECK_EQ(mOMXClient.connect(), (status_t)OK);
    mOMX = mOMXClient.interface();
    mNode = 0;
    mObserver = new OMXDecoderObserver();
    err = mOMX->allocateNode("OMX.TI.DUCATI1.VIDEO.DECODER", mObserver, &mNode);
    if (err != OK) {
        VTC_LOGD("Failed to allocate OMX node!!");
        return -1;
    }
    mObserver->setCodec(this);

    // initialize omx callback handling thread
    if(mOMXCallbackHandler.get() == NULL)
        mOMXCallbackHandler = new OMXCallbackHandler(this);

    if ( NULL == mOMXCallbackHandler.get() ) {
        VTC_LOGE("Couldn't create omx callback handler");
        return -1;
    }

    err = mOMXCallbackHandler->run("OMXCallbackThread", PRIORITY_URGENT_DISPLAY);
    if ( err != NO_ERROR ) {
        if( err == INVALID_OPERATION) {
            VTC_LOGE("omx callback handler thread already runnning!!");
            err = NO_ERROR;
        } else {
            VTC_LOGE("Couldn't run omx callback handler thread");
            return -1;
        }
    }

    // initialize source handling thread
    if(mSourceHandler.get() == NULL)
        mSourceHandler = new SourceHandler(this);

    if ( NULL == mSourceHandler.get() ) {
        VTC_LOGE("Couldn't create source handler");
        return -1;
    }

    err = mSourceHandler->run("SourceThread", PRIORITY_URGENT_DISPLAY);
    if ( err != NO_ERROR ) {
        if( err == INVALID_OPERATION) {
            VTC_LOGE("source handler thread already runnning!!");
            err = NO_ERROR;
        } else {
            VTC_LOGE("Couldn't run source handler thread");
            return -1;
        }
    }

    mState = OMX_StateLoaded;
    waitForStateChange = 0;

    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    INIT_OMX_STRUCT(&format, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    bool found = false;
    OMX_U32 index = 0;
    format.nPortIndex = OUTPUT_PORT;
    format.nIndex = 0;
    for (;;) {
        format.nIndex = index;
        err = mOMX->getParameter(mNode, OMX_IndexParamVideoPortFormat, &format, sizeof(format));
        if (err != OK) {
            VTC_LOGD( "get OMX_IndexParamVideoPortFormat OutPort Error:%d", err);
            return -1;
        }

        if (format.eCompressionFormat == OMX_VIDEO_CodingUnused
                && format.eColorFormat == (OMX_TI_COLOR_FormatYUV420PackedSemiPlanar)) {
            found = true;
            break;
        }
        ++index;
    }

    if (!found) {
        VTC_LOGE("Did not find a match.");
        return -1;
    }

    VTC_LOGV("found a match.");
    err = mOMX->setParameter(mNode, OMX_IndexParamVideoPortFormat, &format, sizeof(format));
    if (err != OK) {
        VTC_LOGD( "set OMX_IndexParamVideoPortFormat OutPort Error:%d", err);
        return -1;
    }

    //
    // Populate Video input Port
    //
    INIT_OMX_STRUCT(&tInPortDef, OMX_PARAM_PORTDEFINITIONTYPE);

    tInPortDef.nPortIndex = INPUT_PORT;
    err = mOMX->getParameter(mNode, OMX_IndexParamPortDefinition, &tInPortDef, sizeof(tInPortDef));
    if (err != OK) {
        VTC_LOGD( "get OMX_IndexParamPortDefinition InPort Error:%d", err);
        return -1;
    }

    tInPortDef.format.video.nFrameWidth = mWidth;
    tInPortDef.format.video.nFrameHeight = mHeight;
    tInPortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
    tInPortDef.nBufferCountActual = 4; // better to match this with the number of encoder output buffers
    tInPortDef.nBufferSize = (mWidth * mHeight);
    err = mOMX->setParameter(mNode, OMX_IndexParamPortDefinition, &tInPortDef, sizeof(tInPortDef));
    if (err != OK) {
        VTC_LOGD( "set OMX_IndexParamPortDefinition InPort Error:%d", err);
        return -1;
    }

    err = mOMX->getParameter(mNode, OMX_IndexParamPortDefinition, &tInPortDef, sizeof(tInPortDef));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamPortDefinition InPort Error:%d", err);
        return -1;
    }

    //
    // Populate Video output Port
    //
    INIT_OMX_STRUCT(&tOutPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    tOutPortDef.nPortIndex = OUTPUT_PORT;
    err = mOMX->getParameter(mNode, OMX_IndexParamPortDefinition,&tOutPortDef, sizeof(tOutPortDef));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamPortDefinition OutPort Error:%d", err);
        return -1;
    }

    tOutPortDef.format.video.nFrameWidth = mWidth;
    tOutPortDef.format.video.nFrameHeight = mHeight;
    tOutPortDef.format.video.nStride = 4096;
    tOutPortDef.format.video.xFramerate = (mFrameRate << 16);
    tOutPortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420PackedSemiPlanar;
//    tOutPortDef.nBufferCountActual += 2; // 2 for surface flinger.
    //set buffer count such that port reconfig can be avoided..
    //is that possible? in any case add 2 for surface flinger.

    err = mOMX->setParameter(mNode, OMX_IndexParamPortDefinition, &tOutPortDef, sizeof(tOutPortDef));
    if (err != OK) {
        VTC_LOGD( "set OMX_IndexParamPortDefinition OutPort Error:%d", err);
        return -1;
    }

    err = mOMX->getParameter(mNode, OMX_IndexParamPortDefinition,&tOutPortDef, sizeof(tOutPortDef));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamPortDefinition OutPort Error:%d", err);
        return -1;
    }

    //
    // setup (code specific) AVC Decoder paramters
    //
    OMX_VIDEO_PARAM_AVCTYPE h264type;
    INIT_OMX_STRUCT(&h264type,OMX_VIDEO_PARAM_AVCTYPE);
    h264type.nPortIndex = INPUT_PORT;

    err = mOMX->getParameter(mNode, OMX_IndexParamVideoAvc, &h264type, sizeof(h264type));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamVideoAvc failed : %d", err);
        return -1;
    }

    h264type.eProfile = profile;
    h264type.eLevel = level;
    h264type.nRefFrames = refFrames;

    err = mOMX->setParameter(mNode, OMX_IndexParamVideoAvc, &h264type, sizeof(h264type));
    if (err != OK) {
        VTC_LOGD("set OMX_IndexParamVideoAvc failed : %d", err);
        return -1;
    }

    // Native Window related calls
    err = mOMX->enableGraphicBuffers(mNode, OUTPUT_PORT, OMX_TRUE);
    if (err != 0) {
        return err;
    }

    android_native_rect_t crop;
    crop.left = 0;
    crop.top = 0;
    crop.right = mWidth + 1;
    crop.bottom = mHeight + 1;

    // We'll ignore any errors here, if the surface is
    // already invalid, we'll know soon enough.
    native_window_set_crop(mNativeWindow.get(), &crop);

    LOG_FUNCTION_NAME_EXIT

    return 0;

}

status_t OMXDecoder::prepare() {
    status_t err;

    LOG_FUNCTION_NAME_ENTRY

    if (setCurrentState(OMX_StateIdle)) {
        return -1;
    }

    INIT_OMX_STRUCT(&tInPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    tInPortDef.nPortIndex = INPUT_PORT;
    err = mOMX->getParameter(mNode, OMX_IndexParamPortDefinition, &tInPortDef, sizeof(tInPortDef));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamPortDefinition InPort Error:%d", err);
        return -1;
    }
    //dump_video_port_values(tInPortDef);

    mDealer = new MemoryDealer((tInPortDef.nBufferCountActual*tInPortDef.nBufferSize), "PLAYBACK_INPUT");
    for (OMX_U32 i = 0; i < tInPortDef.nBufferCountActual; ++i) {
        sp<IMemory> mMem = mDealer->allocate(tInPortDef.nBufferSize);
        CHECK(mMem.get() != NULL);
        IOMX::buffer_id buffer;
        err = mOMX->allocateBufferWithBackup(mNode, INPUT_PORT, mMem, &buffer);
        if (err != OK) {
            VTC_LOGD("OMX_AllocateBuffer for input port index:%d failed:%d",(int)i,err);
        }
        InPortBufferInfo *info = new InPortBufferInfo;
        info->mem = mMem;
        info->b_id = buffer;
        mInputBuffers.push(info);
        mEmptyInputBuffers.push_back(info);
    }
    VTC_LOGD( "Allocated %d Input port Buffers. ", (int)tInPortDef.nBufferCountActual);


    INIT_OMX_STRUCT(&tOutPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    tOutPortDef.nPortIndex = OUTPUT_PORT;
    err = mOMX->getParameter(mNode, OMX_IndexParamPortDefinition, &tOutPortDef, sizeof(tOutPortDef));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamPortDefinition OutPort Error:%d", err);
        return -1;
    }
    //dump_video_port_values(tOutPortDef);

    err = allocateOutputBuffersFromNativeWindow();
    if (err != OK) {
        VTC_LOGD("allocateOutputBuffersFromNativeWindow failed. err:%d", err);
        return -1;
    }

    // now wait until state becomes idle.
    if (waitForStateSet(OMX_StateIdle)) {
        VTC_LOGD("state change to IDLE failed");
        return -1;
    }

    // now transition to exec
    if (setCurrentState(OMX_StateExecuting)) {
        return -1;
    }

    if (waitForStateSet(OMX_StateExecuting)) {
        VTC_LOGD("state change to EXECUTING failed");
        return -1;
    }

    if (restart() != 0) return -1;

    return 0;
}

status_t OMXDecoder::restart() {
    status_t err;
    LOG_FUNCTION_NAME_ENTRY

    // give output buffers to the OMX
    for (int i=0; i<tOutPortDef.nBufferCountActual; i++) {
        if (mOutputBuffers[i].mStatus == OWNED_BY_US) {
            err = mOMX->fillBuffer(mNode, mOutputBuffers[i].b_id);
            if (err != OK) {
                VTC_LOGD("fillBuffer failed:%d", err);
            }
            else VTC_LOGD("called fillBuffer(%d)",i);
        }
    }

    return 0;
}

status_t OMXDecoder::start(MetaData *params) {
    LOG_FUNCTION_NAME_ENTRY
    mAcceptingBuffers = 1;
    return 0;
}

status_t OMXDecoder::stop() {
    status_t err;

    LOG_FUNCTION_NAME_ENTRY

    mAcceptingBuffers = 0;
    // flush all the buffers
    err = mOMX->sendCommand(mNode,OMX_CommandFlush, INPUT_PORT);
    if (err != OK) {
        VTC_LOGD("OMX_CommandFlush for input port (0) failed:%d", err);
    }

    err = mOMX->sendCommand(mNode,OMX_CommandFlush, OUTPUT_PORT);
    if (err != OK) {
        VTC_LOGD("OMX_CommandFlush for output port (1) failed:%d", err);
    }

    // change state to Idle if not already
    if (mState != OMX_StateIdle && mState != OMX_StateLoaded) {
        if (setCurrentState(OMX_StateIdle)) {
            VTC_LOGD("OMX_StateIdle failed");
        }

        if (waitForStateSet(OMX_StateIdle)) {
            VTC_LOGD("state change to IDLE failed");
        }
    }

    // disable ports
    err = mOMX->sendCommand(mNode, OMX_CommandPortDisable, -1);
    if (err != OK) {
        VTC_LOGD("Error in SendCommand()-OMX_CommandPortDisable:");
    } else {
        VTC_LOGD("OMX_CommandPortDisable done");
    }

    // free input buffers
    for (int i=0; i<(int)tInPortDef.nBufferCountActual; i++) {
        err = mOMX->freeBuffer(mNode, INPUT_PORT, mInputBuffers[i]->b_id);
        if( (err != OK)) {
            VTC_LOGD("Free Buffer for Input Port buffer:%d failed:%d",i,err);
        }
    }

    freeOutputBuffers();

    // change state to Loaded
    if (mState != OMX_StateLoaded) {
        if (setCurrentState(OMX_StateLoaded)) {
            VTC_LOGD("OMX_StateLoaded failed");
        }
        if (waitForStateSet(OMX_StateLoaded)) {
            VTC_LOGD("state change to LOADED failed");
        }
    } else {
        VTC_LOGD("It was already OMX_StateLoaded???");
    }


    usleep(5000);

    err = mOMX->freeNode(mNode);
    CHECK_EQ(err, (status_t)OK);

    VTC_LOGD("OMX_FreeHandle completed");

    //Exit and free ref to callback handling thread
    if ( NULL != mOMXCallbackHandler.get() ) {
        Ti::Utils::Message msg;
        msg.command = OMXCallbackHandler::COMMAND_EXIT;
        //Clear all messages pending first
        mOMXCallbackHandler->clearCommandQ();
        mOMXCallbackHandler->put(&msg);
        mOMXCallbackHandler->requestExitAndWait();
        mOMXCallbackHandler.clear();
    }

    //Exit and free ref to source handling thread
    if ( NULL != mSourceHandler.get() ) {
        Ti::Utils::Message msg;
        msg.command = SourceHandler::COMMAND_EXIT;
        //Clear all messages pending first
        mSourceHandler->clearCommandQ();
        mSourceHandler->put(&msg);
        mSourceHandler->requestExitAndWait();
        mSourceHandler.clear();
    }

    destroyPlaybackSurface();

    //what else needs to be freed..

    return 0;
}

sp<MetaData> OMXDecoder::getFormat() {
    return NULL;
}

status_t OMXDecoder::read(MediaBuffer **buffer, const ReadOptions *options) {
    return 0;
}

void OMXDecoder::AcceptEncodedBuffer(void *pBuffer, OMX_U32 nFilledLen, OMX_TICKS nTimeStamp) {
    //VTC_LOGV("AcceptEncodedBuffer - Len = %d", nFilledLen);
    if (mEmptyInputBuffers.empty()) {
        VTC_LOGE("\n\n\n Ran out of input buffers. Dropping Frames.\n\n\n");
        return;
    }

    if (mPortReconfigInProgress) {
        VTC_LOGV( "%s:\t mPortReconfigInProgress.", __FUNCTION__);
        //return;
    }

    while(mPortReconfigInProgress) sleep(1);

    List<InPortBufferInfo*>::iterator iter;
    InPortBufferInfo *info;
    iter = mEmptyInputBuffers.begin();
    info = (InPortBufferInfo *)*iter;
    mEmptyInputBuffers.erase(iter);

    info->nFilledLen = nFilledLen;
    info->nTimeStamp = nTimeStamp;
    memcpy((void*)info->mem->pointer(), pBuffer, nFilledLen);

    Ti::Utils::Message msg;
    msg.command = SourceHandler::COMMAND_PROCESS_MSG;
    msg.arg1 = (void*)info;
    mSourceHandler->put(&msg);

}

OMX_ERRORTYPE OMXDecoder::EventHandler(OMX_EVENTTYPE eEvent, OMX_U32 nData1,OMX_U32 nData2) {
    OMX_ERRORTYPE errorType;
    VTC_LOGV("########## EventHandler: eEvent:0x%x, nData1:0x%x, nData2:0x%x, pid=%d", (int)eEvent, (int)nData1, (int)nData2, getpid());

    switch (eEvent) {
        case OMX_EventCmdComplete:
            if (nData1 == OMX_CommandPortDisable) {
                if (nData2 == OMX_DirInput) {
                    VTC_LOGD( "Component OMX_EventCmdComplete OMX_CommandPortDisable OMX_DirInput");
                }
                if (nData2 == OMX_DirOutput) {
                    VTC_LOGD( "Component OMX_EventCmdComplete OMX_CommandPortDisable OMX_DirOutput");
                    if (mPortReconfigInProgress) {
                        status_t err = mOMX->sendCommand(mNode, OMX_CommandPortEnable, OUTPUT_PORT);
                        CHECK_EQ(err, (status_t)OK);
                        allocateOutputBuffersFromNativeWindow();
                    }
                }
            } else if (nData1 == OMX_CommandPortEnable) {
                if (nData2 == OUTPUT_PORT) {
                    VTC_LOGD( "Component OMX_EventCmdComplete OMX_CommandPortEnable OMX_DirOutput");
                    restart();
                    mPortReconfigInProgress = false;
                    VTC_LOGD("\nPort Reconfiguration completed.. \n");

                }
            } else if (nData1 == OMX_CommandStateSet) {
                VTC_LOGD( "Component OMX_EventCmdComplete OMX_CommandStateSet new State:%s",OMXStateName((OMX_STATETYPE)nData2));
                {
                    Mutex::Autolock autoLock(mLock);
                    mState = (OMX_STATETYPE)nData2;
                    waitForStateChange = 0;
                }
                mAsyncCompletion.signal();

            } else if (nData1 == OMX_CommandFlush) {
                VTC_LOGD( "Component OMX_EventCmdComplete OMX_CommandFlush port:%d",(int)nData2);
            } else {
                VTC_LOGD( "Component OMX_EventCmdComplete command:%d",(int)nData1);
            }
            break;

        case OMX_EventPortSettingsChanged:
            if (nData2 == 0 || nData2 == OMX_IndexParamPortDefinition) {
                VTC_LOGD("\nPort Reconfiguration in progress.. \n");
                mPortReconfigInProgress = true;
                status_t err = mOMX->sendCommand(mNode, OMX_CommandPortDisable, nData1);
                CHECK_EQ(err, (status_t)OK);

                freeOutputBuffers();
            } else if ((nData1 == OUTPUT_PORT)&& (nData2 == OMX_IndexConfigCommonOutputCrop)) {

                OMX_CONFIG_RECTTYPE rect;
                INIT_OMX_STRUCT(&rect, OMX_CONFIG_RECTTYPE);
                rect.nPortIndex = OUTPUT_PORT;
                status_t err = mOMX->getConfig(mNode, OMX_IndexConfigCommonOutputCrop, &rect, sizeof(rect));

                if (err == OK) {
                    VTC_LOGI("Crop rect is %ld x %ld @ (%ld, %ld)", rect.nWidth, rect.nHeight, rect.nLeft, rect.nTop);

                    android_native_rect_t crop;
                    crop.left = rect.nLeft;
                    crop.top = rect.nTop;
                    crop.right = rect.nLeft + rect.nWidth;
                    crop.bottom = rect.nTop + rect.nHeight;

                    native_window_set_crop(mNativeWindow.get(), &crop);
                }
            } else {
                VTC_LOGD("\n\nNOT PROCESSING THIS OMX_EventPortSettingsChanged EVENT: nData1 = 0x%x, nData2 = 0x%x\n\n", nData1, nData2);
            }
            break;

        case OMX_EventError:
            errorType = (OMX_ERRORTYPE) nData1;
            VTC_LOGD( "\n\n\nComponent OMX_EventError error:%x\n\n\n",errorType);
            break;

        default:
            break;
    }

    LOG_FUNCTION_NAME_EXIT
    return OMX_ErrorNone;
}

status_t OMXDecoder::FillBufferDone(OMX_BUFFERHEADERTYPE* pBufferHdr) {
    status_t err;

    OMX_U32 i=0;

    //LOG_FUNCTION_NAME_ENTRY

    // not mAcceptingBuffers, just return, eventually, decoder will stop
    if (mAcceptingBuffers == 0) {
        VTC_LOGV( " in non mAcceptingBuffers mode");
        return OMX_ErrorNone;
    }

    if (mPortReconfigInProgress) {
        VTC_LOGV( "%s:\t mPortReconfigInProgress", __FUNCTION__);
        return OMX_ErrorNone;
    }

    int sz = tOutPortDef.nBufferCountActual;
    for (i = 0; i < sz; i++) {
        if (pBufferHdr == mOutputBuffers[i].b_id) {
            break;
        }
    }

    if (i == sz) {
        VTC_LOGE("FillBufferDone returned unknown buffer header! i=%d",(int)i);
        return -1;
    }
    //VTC_LOGD( "----- %d ----- ", (int)i);
    PortBufferInfo *info = &mOutputBuffers.editItemAt(i);
    info->mStatus = OWNED_BY_US;

    err = mNativeWindow->queueBuffer(mNativeWindow.get(), mOutputBuffers[i].gb.get());
    if (err != 0) {
        VTC_LOGE("queueBuffer failed with error %s (%d)", strerror(-err), -err);
        return -1;
    }
    info->mStatus = OWNED_BY_NATIVE_WINDOW;

    if (mDebugFlags & FPS_DECODER) PrintDecoderFPS();

    ANativeWindowBuffer* buf;
    err = mNativeWindow->dequeueBuffer(mNativeWindow.get(), &buf);
    if (err != 0) {
        VTC_LOGE("dequeueBuffer failed w/ error 0x%08x", err);
        return -1;
    }

    for (i = 0; i < sz; i++) {
        if (mOutputBuffers[i].gb->handle == buf->handle) {
            break;
        }
    }

    if (i == sz) {
        VTC_LOGE("FillBufferDone returned unknown buffer header! i=%d",(int)i);
        return -1;
    }

    info = &mOutputBuffers.editItemAt(i);
    info->mStatus = OWNED_BY_US;

    err = mOMX->fillBuffer(mNode, mOutputBuffers[i].b_id);
    if (err != OK) {
        VTC_LOGE("OMX_FillThisBuffer failed:%d", err);
    }
    info->mStatus = OWNED_BY_COMPONENT;

    //mBufferCount++;
    //VTC_LOGV("EXIT FillBufferDone: nFilledLen=%d, mBufferCount=%d", nFilledLen, mBufferCount);

    return OMX_ErrorNone;
}

status_t OMXDecoder::EmptyBufferDone(OMX_BUFFERHEADERTYPE* pBufferHdr) {
    status_t err;
    OMX_U32 i=0;
    //LOG_FUNCTION_NAME_ENTRY

    // not mAcceptingBuffers, just return, eventually, decoder will stop
    if (mAcceptingBuffers == 0) {
        VTC_LOGV( " in non mAcceptingBuffers mode");
        return OMX_ErrorNone;
    }

    int sz = mInputBuffers.size();
    for (i = 0; i < sz; i++) {
        if (pBufferHdr == mInputBuffers[i]->b_id)
            break;
    }

    if (i == sz) {
        VTC_LOGE("EmptyBufferDone returned unknown buffer header! i=%d",(int)i);
        return -1;
    }

    mEmptyInputBuffers.push_back(mInputBuffers[i]);

    //LOG_FUNCTION_NAME_EXIT

    return err;
}

status_t OMXDecoder::setCurrentState(OMX_STATETYPE newState) {
    VTC_LOGD("Attempting to set state to %s.", OMXStateName(newState));

    status_t err = mOMX->sendCommand(mNode, OMX_CommandStateSet, newState);
    if (err != OK) {
        VTC_LOGD("setCurrentState: Error:%d", err);
        return -1;
    }

    LOG_FUNCTION_NAME_EXIT
    return 0;
}

status_t OMXDecoder::waitForStateSet(OMX_STATETYPE newState) {
    VTC_LOGD("ENTER waitForStateSet: Waiting to move to state %s .....", OMXStateName(newState));

    if (newState == mState) {
        VTC_LOGD("New State [%s] already set!", OMXStateName(newState));
        return 0;
    }

    status_t retval = mAsyncCompletion.waitRelative(mLock, TWO_SECOND);
    if (retval) {
        VTC_LOGD("mAsyncCompletion.waitRelative RETURNED %d", retval);
        if (errno == ETIMEDOUT) {
            VTC_LOGD("$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Waiting for State change timed out $$$$$$$$$$$$$$$$$$$$$$$$$$$$");
            waitForStateChange = 0;
        }
    }

    if (newState == mState) {
        VTC_LOGD("State [%s] Set !!!!!!!!!!!!!!!!!", OMXStateName(newState));
        return 0;
    }

    LOG_FUNCTION_NAME_EXIT
    return -1;
}


status_t OMXDecoder::createPlaybackSurface() {

    mSurfaceComposerClient = new SurfaceComposerClient();
    CHECK_EQ(mSurfaceComposerClient->initCheck(), (status_t)OK);

    mSurfaceControl = mSurfaceComposerClient->createSurface(0,
                                           320, 320, HAL_PIXEL_FORMAT_RGB_565);

    mNativeWindow = mSurfaceControl->getSurface();

    mSurfaceComposerClient->openGlobalTransaction();
    mSurfaceControl->setLayer(0x7fffffff);
    mSurfaceControl->setPosition(10, 10);
    mSurfaceControl->setSize(300, 300);
    mSurfaceControl->show();
    mSurfaceComposerClient->closeGlobalTransaction();

    return 0;
}

status_t OMXDecoder::destroyPlaybackSurface() {

    if ( NULL != mNativeWindow.get() ) {
        mNativeWindow.clear();
    }

    if ( NULL != mSurfaceControl.get() ) {
        mSurfaceControl->clear();
        mSurfaceControl.clear();
    }

    if ( NULL != mSurfaceComposerClient.get() ) {
        mSurfaceComposerClient->dispose();
        mSurfaceComposerClient.clear();
    }

    return 0;
}

status_t OMXDecoder::allocateOutputBuffersFromNativeWindow() {

    status_t err;
    err = mOMX->getParameter(mNode, OMX_IndexParamPortDefinition, &tOutPortDef, sizeof(tOutPortDef));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamPortDefinition OutPort Error:%d", err);
        return -1;
    }

    err = native_window_set_scaling_mode(mNativeWindow.get(),
            NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

    if (err != OK) {
        return err;
    }

    OMX_U32 nBufferCnt = tOutPortDef.nBufferCountActual;
#ifdef NO_PORT_RECONFIG

    VTC_LOGD("\nO/P Buffer Reqmt: %d buffers of size %d x %d\n", tOutPortDef.nBufferCountActual, tOutPortDef.format.video.nFrameWidth, tOutPortDef.format.video.nFrameHeight);

    int newBufferRqmt = tOutPortDef.nBufferCountActual *
            tOutPortDef.format.video.nFrameWidth *
            tOutPortDef.format.video.nFrameHeight *
            3 / 2;
    bool bufferRqmtsChanged = (mSizeOfAllAllocatedOutputBuffers < newBufferRqmt) ? true : false;
    nBufferCnt = (MAX_OUTPUT_BUF_NUM > tOutPortDef.nBufferCountActual) ? MAX_OUTPUT_BUF_NUM : tOutPortDef.nBufferCountActual;

    if ((mPortReconfigInProgress == false)|| bufferRqmtsChanged ) {
        int framewidth = 0;
        int frameheight = 0;

        if (tOutPortDef.format.video.nFrameWidth > MAX_FRAME_WIDTH_720P) {
            framewidth = (tOutPortDef.format.video.nFrameWidth > MAX_FRAME_WIDTH)? tOutPortDef.format.video.nFrameWidth : MAX_FRAME_WIDTH;
        } else {
            framewidth = MAX_FRAME_WIDTH_720P;
        }

        if (tOutPortDef.format.video.nFrameHeight > MAX_FRAME_HEIGHT_720P) {
            frameheight = (tOutPortDef.format.video.nFrameHeight > MAX_FRAME_HEIGHT)? tOutPortDef.format.video.nFrameHeight : MAX_FRAME_HEIGHT;
        } else {
            frameheight = MAX_FRAME_HEIGHT_720P;
        }

        err = native_window_set_buffers_geometry(
                mNativeWindow.get(),
                framewidth,
                frameheight,
                tOutPortDef.format.video.eColorFormat);

        if (err != 0) {
            VTC_LOGE("native_window_set_buffers_geometry failed: %s (%d)",
                    strerror(-err), -err);
            return err;
        }
        mSizeOfAllAllocatedOutputBuffers = nBufferCnt * framewidth * frameheight * 3 / 2;
        VTC_LOGD("\nO/P Buffers actually allocated: %d buffers of size %d x %d\n\n", nBufferCnt, framewidth, frameheight);
    }
    else VTC_LOGI("\n---RECONFIGURING: skip native_window_set_buffers_geometry()\n");

#else
    err = native_window_set_buffers_geometry(
            mNativeWindow.get(),
            tOutPortDef.format.video.nFrameWidth,
            tOutPortDef.format.video.nFrameHeight,
            tOutPortDef.format.video.eColorFormat);

    if (err != 0) {
        VTC_LOGE("native_window_set_buffers_geometry failed: %s (%d)",
                strerror(-err), -err);
        return err;
    }

#endif

    // Set up the native window.
    OMX_U32 usage = 0;
    err = mOMX->getGraphicBufferUsage(mNode, OUTPUT_PORT, &usage);
    if (err != 0) {
        VTC_LOGW("querying usage flags from OMX IL component failed: %d", err);
        // XXX: Currently this error is logged, but not fatal.
        usage = 0;
    }

    VTC_LOGV("native_window_set_usage usage=0x%lx", usage);
    err = native_window_set_usage(
            mNativeWindow.get(), usage | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_EXTERNAL_DISP);
    if (err != 0) {
        VTC_LOGE("native_window_set_usage failed: %s (%d)", strerror(-err), -err);
        return err;
    }

#ifdef NO_PORT_RECONFIG

    if ((mPortReconfigInProgress == false)|| bufferRqmtsChanged ) {
        err = native_window_set_buffer_count(mNativeWindow.get(), nBufferCnt);
        if (err != 0) {
            VTC_LOGE("native_window_set_buffer_count failed: %s (%d)", strerror(-err),
                    -err);
            return err;
        }
        VTC_LOGI("allocating %lu buffers from a native window", nBufferCnt);
    }
    else VTC_LOGI("---RECONFIGURING: skip native_window_set_buffer_count()\n\n");
#else

    err = native_window_set_buffer_count(mNativeWindow.get(), nBufferCnt);
    if (err != 0) {
        VTC_LOGE("native_window_set_buffer_count failed: %s (%d)", strerror(-err),
                -err);
        return err;
    }

    VTC_LOGI("allocating %lu buffers from a native window of size %lu on "
            "output port", tOutPortDef.nBufferCountActual, tOutPortDef.nBufferSize);

#endif

    OMX_U32 i = 0;
    // Dequeue buffers and send them to OMX
    for (i = 0; i < nBufferCnt; i++) {
        ANativeWindowBuffer* buf;
        err = mNativeWindow->dequeueBuffer(mNativeWindow.get(), &buf);
        if (err != 0) {
            VTC_LOGE("dequeueBuffer failed: %s (%d)", strerror(-err), -err);
            break;
        }

        sp<GraphicBuffer> graphicBuffer(new GraphicBuffer(buf, false));
        IOMX::buffer_id bufferId = 0;

        if (i < tOutPortDef.nBufferCountActual) {
            err = mOMX->useGraphicBuffer(mNode, OUTPUT_PORT, graphicBuffer,
                    &bufferId);
            if (err != 0) {
                VTC_LOGE("registering GraphicBuffer with OMX IL component "
                        "failed: %d", err);
                break;
            }
        }

        PortBufferInfo pbi;
        pbi.gb = graphicBuffer;
        pbi.b_id = bufferId;
        pbi.mStatus = OWNED_BY_US;
        mOutputBuffers.push(pbi);

        VTC_LOGI("registered graphic buffer with ID %p (pointer = %p)",
                bufferId, graphicBuffer.get());
    }

    OMX_U32 cancelStart;
    OMX_U32 cancelEnd;
    if (err != 0) {
        // If an error occurred while dequeuing we need to cancel any buffers
        // that were dequeued.
        cancelStart = 0;
        cancelEnd = i;
    } else {
        // Return the last two buffers to the native window.
        cancelStart = tOutPortDef.nBufferCountActual - 2;
        cancelEnd = tOutPortDef.nBufferCountActual;
    }

    for (OMX_U32 i = cancelStart; i < cancelEnd; i++) {
        PortBufferInfo *info = &mOutputBuffers.editItemAt(i);
        err = mNativeWindow->cancelBuffer(
                mNativeWindow.get(), info->gb.get());
        if (err != 0) {
          VTC_LOGE("cancelBuffer failed w/ error 0x%08x", err);
          return err;
        }
        info->mStatus = OWNED_BY_NATIVE_WINDOW;
    }

    return err;
}

status_t OMXDecoder::drainInputBuffer(InPortBufferInfo *info) {
    OMX_TICKS ts;
    ts = info->nTimeStamp;
    if (mDebugFlags & DECODER_LATENCY) ts = systemTime() / 1000;
    status_t err = mOMX->emptyBuffer(mNode, info->b_id, 0, info->nFilledLen, OMX_BUFFERFLAG_ENDOFFRAME, ts);
    if (err != OK) {
        VTC_LOGD("OMX_EmptyThisBuffer failed:%d", err);
        return err;
    }
    return 0;
}

status_t OMXDecoder::freeOutputBuffers() {
    status_t err = 0;

    for (size_t i = mOutputBuffers.size(); i-- > 0;) {

        if (i < tOutPortDef.nBufferCountActual) {
            err = mOMX->freeBuffer(mNode, OUTPUT_PORT, mOutputBuffers[i].b_id);
            if (err != OK) VTC_LOGE("\n\n\n Free Buffer for Output Port buffer:%d failed:%d\n\n\n",i,err);
        }

        // Cancel the buffer if it belongs to an ANativeWindow.
        if (mOutputBuffers[i].mStatus == OWNED_BY_US && mOutputBuffers[i].gb != 0) {
            err = mNativeWindow->cancelBuffer(mNativeWindow.get(), mOutputBuffers[i].gb.get());
            if (err != 0)VTC_LOGE("\n\n\nCancel Buffer for Output Port buffer:%d failed:%d\n\n\n",i,err);
        }

        mOutputBuffers.removeAt(i);
    }

    return 0;
}

