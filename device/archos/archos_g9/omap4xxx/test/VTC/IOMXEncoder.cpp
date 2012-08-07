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
#define LOG_TAG "VTC_ENC"
#define LOG_NDEBUG 0
//#define NO_MEMCOPY 1

using namespace android;

static void PrintEncoderFPS() {
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
        VTC_LOGD("Encoder: %d Frames, %f FPS", mFrameCount, mFps);
    }
    // XXX: mFPS has the value we want
}

static void PrintEncoderLatency(nsecs_t ts) {
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
        VTC_LOGD("Avg Encoder Latency: %d Frames, %llu ms", mFrameCount, avg_latency);
    }
}

static uint64_t get_time_of_day_ms() {
    struct timeval t0;
    gettimeofday(&t0,0);
    return t0.tv_sec*1000 + t0.tv_usec/1000;
}

static void PrintEffectiveBitrate(OMX_U32 filledLen) {
    static int framecount = 0;
    static uint64_t bytecount = 0;
    static uint64_t starttime = get_time_of_day_ms();
    const uint64_t wallclock = get_time_of_day_ms();
    framecount++;
    bytecount+=filledLen;
    int delta=wallclock-starttime;
    if (delta>2000) {
        int fps=framecount*10000/delta;
        const uint64_t br=bytecount*8*1000/delta;
        //VTC_LOGI("ENCODER FPS: %d.%d",fps/10,fps-(fps/10)*10);
        VTC_LOGI("ENCODER EFFECTIVE BITRATE: %llu",br);
        framecount=0;
        bytecount=0;
        starttime = wallclock;
    }
}


bool OMXEncoder::OMXCallbackHandler::Handler() {
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
                mOMXEncoder->on_message(omsg);
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

OMXEncoder::OMXEncoder(const sp<IOMX> &omx, IOMX::node_id node, sp<MyCameraClient> camera, int width, int height, int framerate, int bitrate, char *fname, int sliceHeight):
    mOMX(omx),
    mNode(node),
    mCameraSource(camera) {
    resetParameters(width, height, framerate, bitrate, fname, sliceHeight);
}


OMXEncoder::~OMXEncoder() {
    status_t err = mOMX->freeNode(mNode);
    CHECK_EQ(err, (status_t)OK);
    VTC_LOGD("OMX_FreeHandle completed");
}

status_t OMXEncoder::resetParameters(int width, int height, int framerate, int bitrate, char *fname, int sliceHeight) {
    mWidth = width;
    mHeight = height;
    mFrameRate= framerate;
    mBitRate = bitrate;
    mSliceHeight = sliceHeight;
    mAcceptingBuffers = 0;
    mOutputFD = NULL;
    mBufferCount = 0;
    mOutputBufferCount = 4;
    mCallbackSet = false;
    mState = OMX_StateLoaded;
    return OK;
}

void OMXEncoder::on_message(const omx_message &msg) {
    switch (msg.type) {
        case omx_message::EVENT:
            EventHandler(msg.u.event_data.event, msg.u.event_data.data1, msg.u.event_data.data2);
            break;
        case omx_message::EMPTY_BUFFER_DONE:
            EmptyBufferDone((OMX_BUFFERHEADERTYPE*)msg.u.extended_buffer_data.buffer);
            break;
        case omx_message::FILL_BUFFER_DONE:
            FillBufferDone((OMX_BUFFERHEADERTYPE*)msg.u.extended_buffer_data.buffer,
                    msg.u.extended_buffer_data.range_offset,
                    msg.u.extended_buffer_data.range_length,
                    msg.u.extended_buffer_data.timestamp);
            break;
        default:
            CHECK(!"############ Corrupted Message !!! #############");
            break;
    }
}

status_t OMXEncoder::configure(OMX_VIDEO_AVCPROFILETYPE profile, OMX_VIDEO_AVCLEVELTYPE level, OMX_U32 refFrames) {
    status_t err;
    LOG_FUNCTION_NAME_ENTRY

    VTC_LOGV("\n\nPROFILE=%d\nLEVEL=%d\nRefFrames=%d\nWidth=%d\nHeight=%d\nFramerate=%d\nBitrate=%d\nSliceHeight=%d\n\n",
            profile, level, refFrames, mWidth, mHeight, mFrameRate, mBitRate, mSliceHeight);

    if ((mCallbackSet == false) && ((mDebugFlags & ENCODER_NO_FILE_WRTIE) == 0)) {
        mOutputFD = fopen("/mnt/sdcard/video_0.264","w");
        if (mOutputFD == NULL) {
            VTC_LOGE("\n fopen failed\n");
        }
        VTC_LOGD("\nCallback was NULL. Opened file for writing\n");
    }

    // initialize omx callback handling thread
    if(mOMXCallbackHandler.get() == NULL) {
        mOMXCallbackHandler = new OMXCallbackHandler(this);
    }

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

    mState = OMX_StateLoaded;

    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    INIT_OMX_STRUCT(&format, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    format.nPortIndex = INPUT_PORT;
    format.nIndex = 0;
    bool found = false;
    OMX_U32 index = 0;
    for (;;) {
        format.nIndex = index;
        err = mOMX->getParameter(mNode, OMX_IndexParamVideoPortFormat, &format, sizeof(format));
        if (err != OK) {
            VTC_LOGD( "get OMX_IndexParamVideoPortFormat InPort Error:0x%x. OMX_ErrorUnsupportedIndex=0x%x", err, OMX_ErrorUnsupportedIndex);
            return -1;
        }

        if (format.eCompressionFormat == OMX_VIDEO_CodingUnused
                && format.eColorFormat == (OMX_COLOR_FORMATTYPE)OMX_TI_COLOR_FormatYUV420PackedSemiPlanar) {
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
        VTC_LOGD( "set OMX_IndexParamVideoPortFormat InPort Error:%d", err);
        return -1;
    }

    format.nPortIndex = OUTPUT_PORT;
    format.nIndex = 0;
    found = false;
    index = 0;
    for (;;) {
        format.nIndex = index;
        err = mOMX->getParameter(mNode, OMX_IndexParamVideoPortFormat, &format, sizeof(format));
        if (err != OK) {
            VTC_LOGD( "get OMX_IndexParamVideoPortFormat OutPort Error:%d", err);
            return -1;
        }

        if (format.eCompressionFormat == OMX_VIDEO_CodingAVC
                && format.eColorFormat == (OMX_COLOR_FormatUnused)) {
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
    tInPortDef.format.video.nStride = 4096;
    tInPortDef.format.video.nSliceHeight = mHeight;
    tInPortDef.format.video.xFramerate = (mFrameRate << 16);
    tInPortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
    tInPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_TI_COLOR_FormatYUV420PackedSemiPlanar;
    tInPortDef.nBufferSize = (mWidth * mHeight *3)/2;
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
    tOutPortDef.format.video.xFramerate = 0;
    tOutPortDef.format.video.nBitrate = mBitRate;
    tOutPortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
    tOutPortDef.format.video.eColorFormat = OMX_COLOR_FormatUnused;
    tOutPortDef.nBufferCountActual = mOutputBufferCount;

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
    // setup (code specific) AVC Encoder paramters for OUTPUT port
    //
    OMX_VIDEO_PARAM_AVCTYPE h264type;
    INIT_OMX_STRUCT(&h264type,OMX_VIDEO_PARAM_AVCTYPE);

    h264type.nPortIndex = OUTPUT_PORT;

    err = mOMX->getParameter(mNode, OMX_IndexParamVideoAvc, &h264type, sizeof(h264type));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamVideoAvc failed : %d", err);
        return -1;
    }

    h264type.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;
    h264type.eProfile = profile;
    h264type.eLevel = level;
    h264type.nSliceHeaderSpacing = 0;
    h264type.bUseHadamard = OMX_TRUE;
    h264type.nRefFrames = refFrames;
    h264type.nBFrames = 0;
    //h264type.nPFrames = 30;  // assume iFrameInterval 1, frameRate
    h264type.nPFrames = 0; // Let only the first frame be an I Frame. The rest will be P Frames. For VTC type of applications, you want to insert the IDR only when necessary.
    h264type.nRefIdx10ActiveMinus1 = 0;
    h264type.nRefIdx11ActiveMinus1 = 0;
    h264type.bEntropyCodingCABAC = OMX_FALSE;
    h264type.bWeightedPPrediction = OMX_FALSE;
    h264type.bconstIpred = OMX_FALSE;
    h264type.bDirect8x8Inference = OMX_FALSE;
    h264type.bDirectSpatialTemporal = OMX_FALSE;
    h264type.nCabacInitIdc = 0;
    h264type.bEnableUEP = OMX_FALSE;
    h264type.bEnableFMO = OMX_FALSE;
    h264type.bEnableASO = OMX_FALSE;
    h264type.bEnableRS = OMX_FALSE;
    h264type.bFrameMBsOnly = OMX_TRUE;
    h264type.bMBAFF = OMX_FALSE;
    h264type.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;

    err = mOMX->setParameter(mNode, OMX_IndexParamVideoAvc, &h264type, sizeof(h264type));
    if (err != OK) {
        VTC_LOGD("set OMX_IndexParamVideoAvc failed : %d", err);
        return -1;
    }

    //
    // Set Profile and Level for OUTPUT
    //
    OMX_VIDEO_PARAM_PROFILELEVELTYPE profileLevel;
    INIT_OMX_STRUCT(&profileLevel, OMX_VIDEO_PARAM_PROFILELEVELTYPE);
    profileLevel.nPortIndex = OUTPUT_PORT;
    err = mOMX->getParameter(mNode, OMX_IndexParamVideoProfileLevelCurrent, &profileLevel, sizeof(profileLevel));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamVideoProfileLevelCurrent failed : %d", err);
        return -1;
    }

    profileLevel.eProfile = profile;
    profileLevel.eLevel = level;
    err = mOMX->setParameter(mNode, OMX_IndexParamVideoProfileLevelCurrent, &profileLevel, sizeof(profileLevel));
    if (err != OK) {
        VTC_LOGD("set OMX_IndexParamVideoProfileLevelCurrent failed : %d", err);
        return -1;
    }

    //
    // Set data content type for input port
    //
    OMX_TI_VIDEO_PARAM_FRAMEDATACONTENTTYPE dataContent;
    INIT_OMX_STRUCT(&dataContent, OMX_TI_VIDEO_PARAM_FRAMEDATACONTENTTYPE);
    dataContent.nPortIndex = INPUT_PORT;
    err = mOMX->getParameter(mNode, (OMX_INDEXTYPE)OMX_TI_IndexParamVideoFrameDataContentSettings, &dataContent, sizeof(dataContent));
    if (err != OK) {
        VTC_LOGD("get OMX_TI_IndexParamVideoFrameDataContentSettings failed : %d", err);
        return -1;
    }

    dataContent.eContentType = OMX_TI_Video_Progressive; //appears to be the default value
    err = mOMX->setParameter(mNode, (OMX_INDEXTYPE)OMX_TI_IndexParamVideoFrameDataContentSettings, &dataContent, sizeof(dataContent));
    if (err != OK) {
        VTC_LOGD("set OMX_TI_IndexParamVideoFrameDataContentSettings failed : %d", err);
        return -1;
    }

    //
    // setupBitRate for OUTPUT
    //
    OMX_VIDEO_PARAM_BITRATETYPE bitrateType;
    INIT_OMX_STRUCT(&bitrateType,OMX_VIDEO_PARAM_BITRATETYPE);
    bitrateType.nPortIndex = OUTPUT_PORT;
    err = mOMX->getParameter(mNode, OMX_IndexParamVideoBitrate, &bitrateType, sizeof(bitrateType));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamVideoBitrate failed : %d", err);
        return -1;
    }

    bitrateType.eControlRate = OMX_Video_ControlRateVariable;
    bitrateType.nTargetBitrate = mBitRate;
    err = mOMX->setParameter(mNode, OMX_IndexParamVideoBitrate, &bitrateType, sizeof(bitrateType));
    if (err != OK) {
        VTC_LOGD("set OMX_IndexParamVideoBitrate failed : %d", err);
        return -1;
    }

    err = mOMX->storeMetaDataInBuffers(mNode, INPUT_PORT, OMX_TRUE);
    if (err != OK) {
        VTC_LOGE("Storing meta data in video buffers is not supported");
        return -1;
    }

    if (mSliceHeight == 0) {
        LOG_FUNCTION_NAME_EXIT
        return 0;
    }

    /**************** Configuration specific to Slice based processing ******************/

    //
    // setup data sync mode for INPUT
    //
    OMX_VIDEO_PARAM_DATASYNCMODETYPE syncMode;
    INIT_OMX_STRUCT(&syncMode, OMX_VIDEO_PARAM_DATASYNCMODETYPE);
    syncMode.nPortIndex = INPUT_PORT;
    err = mOMX->getParameter(mNode, (OMX_INDEXTYPE)OMX_TI_IndexParamVideoDataSyncMode, &syncMode, sizeof(syncMode));
    if (err != OK) {
        VTC_LOGD("get OMX_TI_IndexParamVideoDataSyncMode failed : %d", err);
        return -1;
    }

    syncMode.eDataMode = OMX_Video_NumMBRows;
    err = mOMX->setParameter(mNode, (OMX_INDEXTYPE)OMX_TI_IndexParamVideoDataSyncMode, &syncMode, sizeof(syncMode));
    if (err != OK) {
        VTC_LOGD("set OMX_TI_IndexParamVideoDataSyncMode failed: %d", err);
        return -1;
    }

    //
    // setup data sync mode for OUTPUT
    //
    INIT_OMX_STRUCT(&syncMode, OMX_VIDEO_PARAM_DATASYNCMODETYPE);
    syncMode.nPortIndex = OUTPUT_PORT;
    err = mOMX->getParameter(mNode, (OMX_INDEXTYPE)OMX_TI_IndexParamVideoDataSyncMode, &syncMode, sizeof(syncMode));
    if (err != OK) {
        VTC_LOGD("get OMX_TI_IndexParamVideoDataSyncMode failed : %d", err);
        return -1;
    }

    syncMode.eDataMode = OMX_Video_EntireFrame;
    syncMode.nNumDataUnits = 1;
    err = mOMX->setParameter(mNode, (OMX_INDEXTYPE)OMX_TI_IndexParamVideoDataSyncMode, &syncMode, sizeof(syncMode));
    if (err != OK) {
        VTC_LOGD("set OMX_TI_IndexParamVideoDataSyncMode failed: %d", err);
        return -1;
    }

    LOG_FUNCTION_NAME_EXIT
    return 0;
}

status_t OMXEncoder::prepare() {
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

    if (mSliceHeight == 0) { // non tunnel mode

        mDealer[INPUT_PORT] = new MemoryDealer((tInPortDef.nBufferCountActual*tInPortDef.nBufferSize), "RECORD_INPUT");
        for (OMX_U32 i = 0; i < tInPortDef.nBufferCountActual; ++i) {
            mBufferInfo[INPUT_PORT][i].mEncMem= mDealer[INPUT_PORT]->allocate(tInPortDef.nBufferSize);
            CHECK(mBufferInfo[INPUT_PORT][i].mEncMem.get() != NULL);
            err = mOMX->allocateBufferWithBackup(mNode, INPUT_PORT, mBufferInfo[INPUT_PORT][i].mEncMem, (void**)(&(mBufferInfo[INPUT_PORT][i].mBufferHdr)));
            if (err != OK) {
                VTC_LOGE("OMX_AllocateBuffer for input port index:%d failed:%d",(int)i,err);
                mBufferInfo[INPUT_PORT][i].mBufferHdr = NULL;
            }
        }
        VTC_LOGD( "Allocated %d Input port Buffers. ", (int)tInPortDef.nBufferCountActual);
    }

    INIT_OMX_STRUCT(&tOutPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    tOutPortDef.nPortIndex = OUTPUT_PORT;
    err = mOMX->getParameter(mNode, OMX_IndexParamPortDefinition, &tOutPortDef, sizeof(tOutPortDef));
    if (err != OK) {
        VTC_LOGE("get OMX_IndexParamPortDefinition OutPort Error:%d", err);
        return -1;
    }

#ifdef NO_MEMCOPY
    err = allocateOutputBuffers();
    if (err != OK) return -1;
#else

    mDealer[OUTPUT_PORT] = new MemoryDealer((tOutPortDef.nBufferCountActual*tOutPortDef.nBufferSize), "RECORD_OUTPUT");
    for (OMX_U32 i = 0; i < tOutPortDef.nBufferCountActual; ++i) {
        mBufferInfo[OUTPUT_PORT][i].mEncMem = mDealer[OUTPUT_PORT]->allocate(tOutPortDef.nBufferSize);
        err = mOMX->allocateBufferWithBackup(mNode, OUTPUT_PORT, mBufferInfo[OUTPUT_PORT][i].mEncMem, (void**)(&(mBufferInfo[OUTPUT_PORT][i].mBufferHdr)));
        if (err != OK) {
            VTC_LOGD("OMX_UseBuffer for output port index:%d failed:%d",(int)i,err);
            mBufferInfo[OUTPUT_PORT][i].mBufferHdr = NULL;
            return -1;
        }
    }
#endif

    VTC_LOGD( "Allocated %d Output port Buffers. ", (int)tOutPortDef.nBufferCountActual);

    LOG_FUNCTION_NAME_EXIT
    return 0;
}

status_t OMXEncoder::start(MetaData *params) {
    status_t err;

    LOG_FUNCTION_NAME_ENTRY

    // now wait until state becomes idle.
    if (waitForStateSet(OMX_StateIdle)) {
        VTC_LOGD("state change to IDLE failed");
        return -1;
    }

    // If we are going to reuse the node, then port enable is a MUST
    // since we are disabling the port during stop

    // now transition to exec
    if (setCurrentState(OMX_StateExecuting)) {
        return -1;
    }

    if (waitForStateSet(OMX_StateExecuting)) {
        VTC_LOGD("state change to EXECUTING failed");
        return -1;
    }

    mAcceptingBuffers = 1;  // let OMX callbacks to handle buffers

    INIT_OMX_STRUCT(&tInPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    tInPortDef.nPortIndex = INPUT_PORT;
    err = mOMX->getParameter(mNode, OMX_IndexParamPortDefinition, &tInPortDef, sizeof(tInPortDef));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamPortDefinition InPort Error:%d", err);
        return -1;
    }

    INIT_OMX_STRUCT(&tOutPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    tOutPortDef.nPortIndex = OUTPUT_PORT;
    err = mOMX->getParameter(mNode, OMX_IndexParamPortDefinition, &tOutPortDef, sizeof(tOutPortDef));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexParamPortDefinition OutPort Error:%d", err);
        return -1;
    }

    // give output buffers to the OMX
    for (int i=0; i<(int)tOutPortDef.nBufferCountActual; i++) {
        err = mOMX->fillBuffer(mNode, mBufferInfo[OUTPUT_PORT][i].mBufferHdr);
        if (err != OK) {
            VTC_LOGD("OMX_FillThisBuffer failed:%d", err);
        } else {
            VTC_LOGD("called fillBuffer(%d)",i);
        }
    }

    // call camera encoder_is_ready
    mCameraSource->encoderReady();

    if (mSliceHeight) {
        LOG_FUNCTION_NAME_EXIT
        return 0;
    }

    /************** non tunnel / frame mode ***************/

    // now wait for payload to be available. when it does, call OMX_EmptyThisBuffer
    int64_t time[3];
    sp<IMemory> payload[3];

    // wait until camera returns a frame
    for (int i=0; i<3; i++) {
        payload[i] = mCameraSource->getCameraPayload(time[i]);
    }

    for (int i=0; i<3; i++) {
        mBufferInfo[INPUT_PORT][i].mCamMem = payload[i];
        memcpy((uint8_t *)mBufferInfo[INPUT_PORT][i].mEncMem->pointer(),  payload[i]->pointer(), payload[i]->size());
        err = mOMX->emptyBuffer(mNode, mBufferInfo[INPUT_PORT][i].mBufferHdr, 0, payload[i]->size(),  OMX_BUFFERFLAG_ENDOFFRAME, (OMX_TICKS)time);
        if (err != OK) {
            VTC_LOGD("OMX_EmptyThisBuffer failed:%d", err);
        } else {
            VTC_LOGD("Called EmptyThisBuffer[%d] ",i);
        }
    }

    LOG_FUNCTION_NAME_EXIT
    return 0;
}

status_t OMXEncoder::stop() {
    status_t err;

    LOG_FUNCTION_NAME_ENTRY

    // stop mAcceptingBuffers any callbacks
    mAcceptingBuffers = 0;
    mCameraSource->encoderNotReady();

    if (mOutputFD) {
        fclose(mOutputFD);
        mOutputFD = NULL;
    }

    if (mSliceHeight == 0) { // non tunnel mode
        // flush all the buffers since mAcceptingBuffers off they won't be returned.
        err = mOMX->sendCommand(mNode,OMX_CommandFlush, INPUT_PORT);
        if (err != OK) {
            VTC_LOGD("OMX_CommandFlush for input port (0) failed:%d", err);
        }
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

    return 0;
}

status_t OMXEncoder::deinit() {
    status_t err;

    LOG_FUNCTION_NAME_ENTRY

    if (mSliceHeight == 0) { // non tunnel mode
        // free input buffers
        for (int i=0; i<(int)tInPortDef.nBufferCountActual; i++) {
            if (mBufferInfo[INPUT_PORT][i].mBufferHdr) {
                err = mOMX->freeBuffer(mNode, INPUT_PORT, mBufferInfo[INPUT_PORT][i].mBufferHdr);
                if( (err != OK)) {
                    VTC_LOGD("Free Buffer for Input Port buffer:%d failed:%d",i,err);
                }
            }
        }
    }

    // free output buffers
    for (int i=0; i <(int)tOutPortDef.nBufferCountActual; i++) {
        if (mBufferInfo[OUTPUT_PORT][i].mBufferHdr) {
            err = mOMX->freeBuffer(mNode,OUTPUT_PORT,mBufferInfo[OUTPUT_PORT][i].mBufferHdr);
            if( (err != OK)) {
                VTC_LOGD("Free Buffer for Output Port buffer:%d failed:%d",i,err);
            }
        }
    }

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

    LOG_FUNCTION_NAME_EXIT
    return 0;
}

sp<MetaData> OMXEncoder::getFormat() {
    return NULL;
}

status_t OMXEncoder::read(MediaBuffer **buffer, const ReadOptions *options) {
    return 0;
}


OMX_ERRORTYPE OMXEncoder::EventHandler(OMX_EVENTTYPE eEvent, OMX_U32 nData1,OMX_U32 nData2) {
    OMX_ERRORTYPE errorType;
    VTC_LOGV("########## EventHandler: eEvent:0x%x, nData1:%d, nData2:%d, pid=%d", (int)eEvent, (int)nData1, (int)nData2, getpid());

    switch (eEvent) {
        case OMX_EventCmdComplete:
            if (nData1 == OMX_CommandPortDisable) {
                if (nData2 == OMX_DirInput) {
                    VTC_LOGD( "Component OMX_EventCmdComplete OMX_CommandPortDisable OMX_DirInput");
                }
                if (nData2 == OMX_DirOutput) {
                    VTC_LOGD( "Component OMX_EventCmdComplete OMX_CommandPortDisable OMX_DirOutput");
                }
            } else if (nData1 == OMX_CommandStateSet) {
                VTC_LOGD( "Component OMX_EventCmdComplete OMX_CommandStateSet new State:%s",OMXStateName((OMX_STATETYPE)nData2));
                {
                    Mutex::Autolock autoLock(mLock);
                    mState = (OMX_STATETYPE)nData2;
                }
                mAsyncCompletion.signal();

            } else if (nData1 == OMX_CommandFlush) {
                VTC_LOGD( "Component OMX_EventCmdComplete OMX_CommandFlush port:%d",(int)nData2);
            } else {
                VTC_LOGD( "Component OMX_EventCmdComplete command:%d",(int)nData1);
            }
            break;

        case OMX_EventError:
            errorType = (OMX_ERRORTYPE) nData1;
            VTC_LOGD( "\n\n\nComponent OMX_EventError error:%x\n\n\n",errorType);
            break;
        default:
            break;
    }
    VTC_LOGV("EXIT EventHandler");
    return errorType;
}

status_t OMXEncoder::FillBufferDone(OMX_BUFFERHEADERTYPE* pBufferHdr, OMX_U32 nOffset, OMX_U32 nFilledLen, OMX_TICKS nTimeStamp) {
    status_t err;
    OMX_U32 i=0;

    // not mAcceptingBuffers, just return, eventually, decoder will stop
    if (mAcceptingBuffers == 0) {
        VTC_LOGV( " in non mAcceptingBuffers mode");
        return OMX_ErrorNone;
    }

    for (i = 0; i < tOutPortDef.nBufferCountActual; i++) {
        if (pBufferHdr == mBufferInfo[OUTPUT_PORT][i].mBufferHdr) {
            break;
        }
    }
    if (i == tOutPortDef.nBufferCountActual) {
        VTC_LOGE("FillBufferDone returned unknown buffer header! i=%d",(int)i);
        return -1;
    }
    //VTC_LOGD( "----- %d ----- ", (int)i);

    if (mDebugFlags & DEBUG_DUMP_ENCODER_TIMESTAMP) VTC_LOGD("FBD TS: %lld", nTimeStamp);

    if (mDebugFlags & FPS_ENCODER) PrintEncoderFPS();

    if (mDebugFlags & ENCODER_LATENCY) PrintEncoderLatency(nTimeStamp);

    if (mDebugFlags & ENCODER_EFFECTIVE_BITRATE) PrintEffectiveBitrate(nFilledLen);

    if (mCallbackSet) {
        mEncodedBufferCallback((mBufferInfo[OUTPUT_PORT][i].mEncMem->pointer() + nOffset), nFilledLen, nTimeStamp);
    } else {
        if (mOutputFD != NULL) {
            i = fwrite((unsigned char *)(mBufferInfo[OUTPUT_PORT][i].mEncMem->pointer() + nOffset), 1, nFilledLen, mOutputFD);
            if (i != nFilledLen) {
                VTC_LOGD("fwrite failed:%d should have been:%d\n", i, nFilledLen);
                return -1;
            }
            fflush(mOutputFD);
        }
    }

    err = mOMX->fillBuffer(mNode, pBufferHdr);
    if (err != OK) {
        VTC_LOGE("OMX_FillThisBuffer failed:%d", err);
    }

    mBufferCount++;
    VTC_LOGV("EXIT FillBufferDone: nOffset: %d, nFilledLen=%d, mBufferCount=%d", nOffset, nFilledLen, mBufferCount);
    return OMX_ErrorNone;
}

status_t OMXEncoder::EmptyBufferDone(OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    status_t err;
    OMX_U32 i=0;
    //VTC_LOGV("ENTER EmptyBufferDone");

    // not mAcceptingBuffers, just return, eventually, decoder will stop
    if (mAcceptingBuffers == 0) {
        VTC_LOGV( " in non mAcceptingBuffers mode");
        return OMX_ErrorNone;
    }

    for (i = 0; i < tInPortDef.nBufferCountActual; i++) {
        if (pBufferHdr == mBufferInfo[INPUT_PORT][i].mBufferHdr) break;
    }

    if (i == tInPortDef.nBufferCountActual) {
        VTC_LOGE("EmptyBufferDone returned unknown buffer header! i=%d",(int)i);
        return -1;
    }

    mCameraSource->releaseBuffer(mBufferInfo[INPUT_PORT][i].mCamMem);

    if (mSliceHeight == 0) { // non tunnel mode

        // now get the next buffer and feed the encoder
        sp<IMemory> payload;
        int64_t time;

        // wait til buffer in the camera
        payload = mCameraSource->getCameraPayload(time);
        if (payload != NULL) {
            mBufferInfo[INPUT_PORT][i].mCamMem = payload;
            memcpy((uint8_t *)mBufferInfo[INPUT_PORT][i].mEncMem->pointer(),  payload->pointer(), payload->size());
            err = mOMX->emptyBuffer(mNode, mBufferInfo[INPUT_PORT][i].mBufferHdr, 0, payload->size(),  OMX_BUFFERFLAG_ENDOFFRAME, time);
            if (err != OK) {
                VTC_LOGE("OMX_EmptyThisBuffer failed:%d", err);
            }
        }
    }

    //VTC_LOGV("EXIT EmptyBufferDone");
    return err;
}

status_t OMXEncoder::setCurrentState(OMX_STATETYPE newState) {
    VTC_LOGV("Attempting to set state to %s.", OMXStateName(newState));

    status_t err = mOMX->sendCommand(mNode, OMX_CommandStateSet, newState);
    if (err != OK) {
        VTC_LOGD("setCurrentState: Error:%d", err);
        return -1;
    }

    LOG_FUNCTION_NAME_EXIT
    return 0;
}

status_t OMXEncoder::waitForStateSet(OMX_STATETYPE newState) {
    VTC_LOGV("waitForStateSet: Waiting to move to state %s .....", OMXStateName(newState));

    if (newState == mState) {
        VTC_LOGD("New State [%s] already set!", OMXStateName(newState));
        return 0;
    }

    status_t retval = mAsyncCompletion.waitRelative(mLock, TWO_SECOND);
    if (retval) {
        if (errno == ETIMEDOUT) {
            VTC_LOGD("$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Waiting for State change timed out $$$$$$$$$$$$$$$$$$$$$$$$$$$$");
        } else {
            VTC_LOGD("$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Waiting for State errno :%d, retval:%d \n", errno, retval);
        }
    }

    if (newState == mState) {
        VTC_LOGD("State [%s] Set !!!!!!!!!!!!!!!!!", OMXStateName(newState));
        return 0;
    }

    LOG_FUNCTION_NAME_EXIT
    return -1;
}

//Function to set the slice mode for video encoder output port
status_t OMXEncoder::setEncoderOutputSlice(OMX_U32 nHeight, OMX_U32 nWidth, OMX_U32 sizeBytes, OMX_U32 sizeMB) {
    status_t err = 0;

    VTC_LOGV("Setting Video Output Slice Mode Size in Bytes:%d, Size in MB:%d\n",sizeBytes, sizeMB);

    if ((!sizeBytes) && (!sizeMB)) {
        //Enc o/p slice not set
        return err;
    }

    if (sizeBytes) {

        if (nWidth <= 320) {
            VTC_LOGD ("Setting the Video Encoder output slice mode NOT supported for given Resolution(width should be > 320)\n");
            return err;
        }

        if (sizeBytes < 256) {
            VTC_LOGD ("Slice size provided for Video Encoder output port too small, should be atleast 256 bytes\n");
            return err;
        }

        OMX_VIDEO_CONFIG_SLICECODINGTYPE slicetype;
        INIT_OMX_STRUCT(&slicetype, OMX_VIDEO_CONFIG_SLICECODINGTYPE);
        slicetype.nPortIndex = OUTPUT_PORT;

        err = mOMX->getConfig(
                mNode, (OMX_INDEXTYPE)OMX_TI_IndexConfigSliceSettings, &slicetype, sizeof(slicetype));
        if (err != OK) {
            VTC_LOGD("get OMX_TI_IndexConfigSliceSettings failed : 0x%x", err);
            return -1;
        }

        slicetype.eSliceMode = OMX_VIDEO_SLICEMODE_AVCByteSlice;
        slicetype.nSlicesize = sizeBytes;

        err = mOMX->setConfig(
                mNode, (OMX_INDEXTYPE)OMX_TI_IndexConfigSliceSettings, &slicetype, sizeof(slicetype));
        if (err != OK) {
            VTC_LOGD("set OMX_TI_IndexConfigSliceSettings OMX_VIDEO_SLICEMODE_AVCByteSlice failed : 0x%x", err);
            return -1;
        }

    } else if (sizeMB) {

        if (sizeMB <= 6) {
            VTC_LOGD ("Macro Block set for the Video Encoder output slice mode NOT supported (very low should be > 6) \n");
            return err;
        }

        /* Max # of MB
           1080p=8160
           720p=3600
           VGA=1200
        */
        if (sizeMB > (((nWidth+15)>> 4) * ((nHeight+15)>> 4))) {
            VTC_LOGD ("Macro Block set for the Video Encoder output slice mode is too large, should be less then \
                    (((PreviewWidth+15)>> 4) * ((PreviewHeight+15)>> 4)) \n");
            return err;
        }

        OMX_VIDEO_CONFIG_SLICECODINGTYPE slicetype;
        INIT_OMX_STRUCT(&slicetype, OMX_VIDEO_CONFIG_SLICECODINGTYPE);
        slicetype.nPortIndex = OUTPUT_PORT;

        err = mOMX->getConfig(
                mNode, (OMX_INDEXTYPE)OMX_TI_IndexConfigSliceSettings, &slicetype, sizeof(slicetype));
        if (err != OK) {
            VTC_LOGD("get OMX_TI_IndexConfigSliceSettings failed : 0x%x", err);
            return -1;
        }

        slicetype.eSliceMode = OMX_VIDEO_SLICEMODE_AVCMBSlice;
        slicetype.nSlicesize = sizeMB;

        err = mOMX->setConfig(
                mNode, (OMX_INDEXTYPE)OMX_TI_IndexConfigSliceSettings, &slicetype, sizeof(slicetype));
        if (err != OK) {
            VTC_LOGD("set OMX_TI_IndexConfigSliceSettings OMX_VIDEO_SLICEMODE_AVCMBSlice failed : 0x%x", err);
            return -1;
        }

    }

    /* Other limitations:
       - Input content type should be progressive (currently nPFrames = 0 by default)
       - Changing parameters at run time will not have effect until next I-frame (hence setting IDR frame forcefully below)
       - Incase of doing the initial setting of nPFrames = 0 (only initial frame is I-frame and all others P-frames),
         you must request an I-frame to the codec after you have set nSlicesize to see your changes take place.
    */

    VTC_LOGV ("Insert IDR frame for the Encoder o/p slice mode setting to get effective \n");
    //Insert IDR frame for the setting to get effective
    OMX_CONFIG_INTRAREFRESHVOPTYPE voptype;
    INIT_OMX_STRUCT(&voptype, OMX_CONFIG_INTRAREFRESHVOPTYPE);
    voptype.nPortIndex = OUTPUT_PORT;

    err = mOMX->getConfig(
            mNode, OMX_IndexConfigVideoIntraVOPRefresh, &voptype, sizeof(voptype));
    if (err != OK) {
        VTC_LOGD("get OMX_IndexConfigVideoIntraVOPRefresh failed : %d", err);
        return -1;
    }

    voptype.IntraRefreshVOP = OMX_TRUE;
    err = mOMX->setConfig(
            mNode, OMX_IndexConfigVideoIntraVOPRefresh, &voptype, sizeof(voptype));
    if (err != OK) {
        VTC_LOGD("set OMX_IndexConfigVideoIntraVOPRefresh failed : %d", err);
        return -1;
    }

    VTC_LOGV("Insert IDR Frame DONE!!!, Setting Video Output Slice Mode\n");
    return err;
}



void OMXEncoder::setCallback(EncodedBufferCallback fp) {
    mEncodedBufferCallback = fp;
    mCallbackSet = true;
}

status_t OMXEncoder::changeFrameRate(int framerate) {
    VTC_LOGV("setConfigVideoFrameRate: %d", framerate);
    OMX_CONFIG_FRAMERATETYPE framerateType;
    INIT_OMX_STRUCT(&framerateType, OMX_CONFIG_FRAMERATETYPE);
    framerateType.nPortIndex = INPUT_PORT;

    status_t err = mOMX->getConfig(
            mNode, OMX_IndexConfigVideoFramerate,
            &framerateType, sizeof(framerateType));
    if (err != OK) {
        return BAD_VALUE;
    }

    framerateType.xEncodeFramerate = framerate << 16;
    err = mOMX->setConfig(mNode, OMX_IndexConfigVideoFramerate, &framerateType, sizeof(framerateType));
    if (err != OK) {
        return BAD_VALUE;
    }
    return OK;
}

status_t OMXEncoder::changeBitRate(int bitrate) {
    OMX_VIDEO_CONFIG_BITRATETYPE bitrateType;
    INIT_OMX_STRUCT(&bitrateType,OMX_VIDEO_CONFIG_BITRATETYPE);
    bitrateType.nPortIndex = OUTPUT_PORT;
    status_t err = mOMX->getConfig(mNode, OMX_IndexConfigVideoBitrate, &bitrateType, sizeof(bitrateType));
    if (err != OMX_ErrorNone) {
      VTC_LOGE("get OMX_IndexConfigVideoBitrate failed err:%X", err);
      return err;
    }

    VTC_LOGD("\nSet encoder bitrate to %d.\n\n", bitrate);
    bitrateType.nEncodeBitrate = bitrate;
    err = mOMX->setConfig(mNode, OMX_IndexConfigVideoBitrate, &bitrateType, sizeof(bitrateType));
    if (err != OMX_ErrorNone) {
      VTC_LOGE("set OMX_IndexConfigVideoBitrate failed error:%x", err);
      return err;
    }

    return OK;
}

#ifdef NO_MEMCOPY
status_t OMXEncoder::allocateOutputBuffer() {
    //USE_ION_BUFFERS_ALLOCATED_BY_DOMX

    int ion_fd = ion_open();
    if(ion_fd == 0) {
        VTC_LOGE("ion_open failed!!!");
        return -1;
    }

    struct ion_handle *importedHandle = NULL;

    for (OMX_U32 i = 0; i < tOutPortDef.nBufferCountActual; ++i) {

        IOMX::buffer_id buffer;
        void *pBuffer;
        err = mOMX->allocateBuffer(mNode, OUTPUT_PORT, tOutPortDef.nBufferSize, &buffer, &pBuffer);
        if (err != OK) {
            VTC_LOGD("OMX_UseBuffer for output port index:%d failed:%d",(int)i,err);
            mBufferInfo[OUTPUT_PORT][i].mBufferHdr = NULL;
        }

        OMX_TI_ION_SHARE_FD shareFDParam;
        INIT_OMX_STRUCT(&shareFDParam, OMX_TI_ION_SHARE_FD);
        shareFDParam.nPortIndex = OUTPUT_PORT;
        shareFDParam.nBufferIndex = i;
        err = mOMX->getParameter(mNode, (OMX_INDEXTYPE)OMX_TI_IndexIONBufferShareHandle, (void*)&shareFDParam, sizeof(shareFDParam));
        if (err != OK) {
            VTC_LOGD("get OMX_TI_IndexIONBufferShareHandle Error:%d", err);
            return -1;
        }
        VTC_LOGD("ENC SHARE FD = %d", shareFDParam.nShareFD);

        OMX_U32 nSize = (tOutPortDef.nBufferSize + LINUX_PAGE_SIZE - 1) & ~(LINUX_PAGE_SIZE - 1);

        err = ion_import(ion_fd, shareFDParam.nShareFD, &importedHandle);
        if (err != OK) {
            VTC_LOGD("ion_import failed. ret = %d", err);
            return -1;
        }
        VTC_LOGD("IMPORT SUCCEEDED");

        int mmap_fd;
        void *pIONBuffer;
        err = ion_map(ion_fd, importedHandle, nSize, PROT_READ | PROT_WRITE, MAP_SHARED, 0, (unsigned char**)&pIONBuffer, &mmap_fd);
        if (err) {
            VTC_LOGE("\n\n$$$$$$$$$$$$$$$$ Userspace mapping of ION buffers returned error %d\n\n", err);
            err = ion_free(ion_fd, h);
            if (err) VTC_LOGE("\n ion_free failed err=%d.\n\n%s\n\n", err, strerror(errno));
            return -1;
        }
        // TODO: More work needs to be done here.. This is just a skeleton for the moment.
    }

    return OK;
}

#endif


