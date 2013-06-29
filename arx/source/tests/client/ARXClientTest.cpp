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

#include <arx_debug.h>
#include <arx/ARXProperties.h>
#include <arx/ARXBufferTypes.h>
#include <arx/ARXFlatBuffer.h>
#include <arx/ARXFlatBufferMgr.h>

#include <ARXImgRenderer.h>
#include <FDRenderer.h>
#include <TestPropListener.h>
#include <ARXClientTest.h>
#include <DVPTestGraph.h>

namespace tiarx {

ARXClientTest::ARXClientTest()
{
    mArx = NULL;
    mPropL = NULL;
    event_init(&mDoneEvt, false_e);

    mCamWidth = 640;
    mCamHeight = 480;
    mSecCamWidth = 320;
    mSecCamHeight = 240;

    mCamId = 0;
    mCamFPS = 30;

    mCamMirror = CAM_MIRROR_NONE;
    mSecCamMirror = CAM_MIRROR_NONE;
    mEnableSec = true;
    mTimeToRun = 0xFFFFFFFF;
    mComputeBufOut = false;

    mCamRend[0] = NULL;
    mCamRend[1] = NULL;

    mUseDVP = false;
}

ARXClientTest::~ARXClientTest()
{
    if (mArx) {
        mArx->destroy();
    }

    delete mCamRend[0];
    delete mCamRend[1];
    delete mPropL;
    event_deinit(&mDoneEvt);
}

arxstatus_t ARXClientTest::parseOptions(int argc, char *argv[])
{
    int numOptions = 11;
    option_t *opts = (option_t *)calloc(numOptions, sizeof(option_t));
    if (opts == NULL) {
        return NOMEMORY;
    }

    for (int i = 0; i < numOptions; i++) {
        opts[i].type = OPTION_TYPE_INT;
        opts[i].size = sizeof(int);
    }

    int index = 0;
    opts[index].datum = &mCamWidth;
    opts[index++].short_switch = "-w";

    opts[index].datum = &mCamHeight;
    opts[index++].short_switch = "-h";

    opts[index].datum = &mSecCamWidth;
    opts[index++].short_switch = "-cw";

    opts[index].datum = &mSecCamHeight;
    opts[index++].short_switch = "-ch";

    opts[index].datum = &mCamId;
    opts[index++].short_switch = "-c";

    opts[index].datum = &mCamFPS;
    opts[index++].short_switch = "-fps";

    opts[index].datum = &mCamMirror;
    opts[index++].short_switch = "-mir";

    opts[index].datum = &mSecCamMirror;
    opts[index++].short_switch = "-mir2";

    opts[index].datum = &mEnableSec;
    opts[index++].short_switch = "-rc2";

    opts[index].datum = &mTimeToRun;
    opts[index++].short_switch = "-t";

    opts[index].datum = &mComputeBufOut;
    opts[index++].short_switch = "-o";

    option_process(argc, argv, opts, numOptions);

    free(opts);

    return NOERROR;
}

arxstatus_t ARXClientTest::init()
{
    mCamRend[0] = new ARXImgRenderer();
    mCamRend[1] = new ARXImgRenderer();
    if (mCamRend[0] == NULL || mCamRend[1] == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate camera renderer!");
        return NOMEMORY;
    }

    mPropL = new TestPropListener(&mDoneEvt);
    if (mPropL == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to create a test property listener!");
        return FAILED;
    }

    mArx = ARAccelerator::create(mPropL, mUseDVP);
    if (mArx == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to create an instance of AR Accelerator!");
        return FAILED;
    }

    arxstatus_t ret = loadEngine();
    if (ret != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to load ARX engine!");
        return ret;
    }

    ret = mCamRend[0]->init(mCamWidth, mCamHeight, 0, 0, BUFF_CAMOUT, mArx, false);
    if (mEnableSec) {
        ret = mCamRend[1]->init(mSecCamWidth, mSecCamHeight, mCamWidth, 0, BUFF_CAMOUT2, mArx, true, mComputeBufOut);
    }

    ret = mArx->setProperty(PROP_CAM_ID, mCamId);
    ret = mArx->setProperty(PROP_CAM_FPS, mCamFPS);
    ret = mArx->setProperty(PROP_CAM_MAIN_MIRROR, mCamMirror);
    ret = mArx->setProperty(PROP_CAM_SEC_MIRROR, mSecCamMirror);

    return ret;
}

arxstatus_t ARXClientTest::loadEngine()
{
    return NOERROR;
}

arxstatus_t ARXClientTest::start()
{
    return mArx->setProperty(PROP_ENGINE_STATE, ENGINE_STATE_START);
}

void ARXClientTest::waitToComplete()
{
    uint32_t timeout = (mTimeToRun == (int)0xFFFFFFFF) ? EVENT_FOREVER : mTimeToRun*1000;
    event_wait(&mDoneEvt, timeout);
}


//=============================================================================
ARXMainClientTest::ARXMainClientTest()
{
    mEnableFD = false_e;
    mEnableGrad = false_e;
    mEnableHarris = false_e;
    mEnableSobel = false_e;
    mEnableFDSync = false_e;
    mEnableFP = false_e;
    mLogFP = false_e;
    mEnableFPFilter = false_e;
    mConfThresh = 200;
    mMinMove = 0;
    mMaxMove = 0xFFFF;

    for (uint32_t i = 0; i < dimof(mStreamRend); i++) {
        mStreamRend[i] = NULL;
    }
    mFdRend = NULL;
    mFPRend = NULL;
}

ARXMainClientTest::~ARXMainClientTest()
{
    if (mArx) {
        mArx->destroy();
        mArx = NULL;
    }
    for (uint32_t i = 0; i < dimof(mStreamRend); i++) {
        delete mStreamRend[i];
    }

    delete mFdRend;
    delete mFPRend;
    ARX_PRINT(ARX_ZONE_ALWAYS, "~ARXMainClientTest");
}

arxstatus_t ARXMainClientTest::init()
{
    arxstatus_t ret = ARXClientTest::init();
    if (ret != NOERROR) {
        return ret;
    }

    for (uint32_t i = 0; i < dimof(mStreamRend); i++) {
        mStreamRend[i] = new ARXImgRenderer();
        if (mStreamRend[i] == NULL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate stream renderer!");
            return NOMEMORY;
        }
    }

    mFdRend = new FDRenderer(mEnableFDSync);
    if (mFdRend == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate face detect renderer!");
        return NOMEMORY;
    }

    mFPRend = new FDRenderer(mEnableFDSync);
    if (mFPRend == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate face parts renderer!");
        return NOMEMORY;
    }

    if (mEnableSobel) {
        ret = mStreamRend[0]->init(mSecCamWidth, mSecCamHeight, mCamWidth, mSecCamHeight, BUFF_SOBEL_3X3, mArx);
    }

    if (mEnableHarris) {
        ret = mStreamRend[1]->init(mSecCamWidth, mSecCamHeight, 0, mCamHeight, BUFF_HARRIS_SCORE, mArx);
    }

    if (mEnableGrad) {
        ret = mStreamRend[2]->init(mSecCamWidth, mSecCamHeight, mSecCamWidth, mCamHeight, BUFF_CANNY2D_GRADIENT_X, mArx);
        ret = mStreamRend[3]->init(mSecCamWidth, mSecCamHeight, 2*mSecCamWidth, mCamHeight, BUFF_CANNY2D_GRADIENT_Y, mArx);
    }

    if (mEnableFD) {
        ret = mFdRend->init(mCamWidth, mCamHeight, 0, 0, mArx);
    }

    if (mEnableFP) {
        ret = mFPRend->init(mCamWidth, mCamHeight, 0, 0, mArx, BUFF_FACEINFO, mLogFP);
        mArx->setProperty(PROP_ENGINE_FPD_FILTER, mEnableFPFilter ? 1 : 0);
        mArx->setProperty(PROP_ENGINE_FPD_CONFTHRESH, mConfThresh);
        mArx->setProperty(PROP_ENGINE_FPD_MINMOVE, mMinMove);
        mArx->setProperty(PROP_ENGINE_FPD_MAXMOVE, mMaxMove);
    }

    return ret;
}

arxstatus_t ARXMainClientTest::parseOptions(int argc, char *argv[])
{
    arxstatus_t ret = ARXClientTest::parseOptions(argc, argv);
    if (ret != NOERROR) {
        return ret;
    }

    int numOptions = 11;
    option_t *opts = (option_t *)calloc(numOptions, sizeof(option_t));
    if (opts == NULL) {
        return NOMEMORY;
    }

    for (int i = 0; i < numOptions; i++) {
        opts[i].type = OPTION_TYPE_BOOL;
        opts[i].size = sizeof(bool_e);
    }

    int index = 0;
    opts[index].datum = &mEnableSobel;
    opts[index++].short_switch = "-rs";

    opts[index].datum = &mEnableFD;
    opts[index++].short_switch = "-rf";

    opts[index].datum = &mEnableFDSync;
    opts[index++].short_switch = "-fdsync";

    opts[index].datum = &mEnableHarris;
    opts[index++].short_switch = "-rh";

    opts[index].datum = &mEnableGrad;
    opts[index++].short_switch = "-rg";

    opts[index].datum = &mEnableFP;
    opts[index++].short_switch = "-rp";

    opts[index].datum = &mLogFP;
    opts[index++].short_switch = "-wp";

    opts[index].datum = &mEnableFPFilter;
    opts[index++].short_switch = "-fpdf";

    opts[index].datum = &mConfThresh;
    opts[index].type = OPTION_TYPE_INT;
    opts[index].size = sizeof(int);
    opts[index++].short_switch = "-conf";

    opts[index].datum = &mMinMove;
    opts[index].type = OPTION_TYPE_INT;
    opts[index].size = sizeof(int);
    opts[index++].short_switch = "-mnm";

    opts[index].datum = &mMaxMove;
    opts[index].type = OPTION_TYPE_INT;
    opts[index].size = sizeof(int);
    opts[index++].short_switch = "-mxm";

    option_process(argc, argv, opts, numOptions);

    free(opts);

    ARX_PRINT(ARX_ZONE_ALWAYS, "FD:%d, FDSync:%d, Sobel:%d, Harris:%d, Gradient:%d, Facial Parts:%d, Confidence Threshold:%d, Min Move:%d, Max Move:%d",
            mEnableFD,
            mEnableFDSync,
            mEnableSobel,
            mEnableHarris,
            mEnableGrad,
            mEnableFP,
            mConfThresh,
            mMinMove,
            mMaxMove);
    return NOERROR;
}

//=============================================================================
class CamPoseListener : public ARXFlatBufferListener
{
public:
    void onBufferChanged(ARXFlatBuffer *pBuffer)
    {
        ARXCameraPoseMatrix *m = reinterpret_cast<ARXCameraPoseMatrix *>(pBuffer->data());

        ARX_PRINT(ARX_ZONE_ALWAYS, "rx[%0.2f %0.2f %0.2f] ry[%0.2f %0.2f %0.2f] rz[%0.2f %0.2f %0.2f] t[%1.2f %1.2f %1.2f]",
                m->matrix[0], m->matrix[4], m->matrix[8],
                m->matrix[1], m->matrix[5], m->matrix[9],
                m->matrix[2], m->matrix[6], m->matrix[10],
                m->matrix[3], m->matrix[7], m->matrix[11]);

        pBuffer->release();
    }
};

ARXCamPoseTest::ARXCamPoseTest()
{
    mCamPoseL = NULL;
}

ARXCamPoseTest::~ARXCamPoseTest()
{
    if (mArx) {
        mArx->destroy();
        mArx = NULL;
    }

    delete mCamPoseL;
}

arxstatus_t ARXCamPoseTest::loadEngine()
{
    return mArx->loadEngine("arxpose");
}

arxstatus_t ARXCamPoseTest::init()
{
    arxstatus_t ret = ARXClientTest::init();
    if (ret != NOERROR) {
        return ret;
    }

    mCamPoseL = new CamPoseListener();
    if (mCamPoseL == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate cam pose listener!");
        return NOMEMORY;
    }

    ARXFlatBufferMgr *mgr = mArx->getFlatBufferMgr(BUFF_CAMERA_POSE);
    if (mgr == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to obtain cam pose stream!");
        return FAILED;
    }

    ret = mgr->registerClient(mCamPoseL);
    if (ret != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to register cam pose listener!");
        return ret;
    }

    return NOERROR;
}

arxstatus_t ARXCamPoseTest::parseOptions(int argc, char *argv[])
{
    arxstatus_t ret = ARXClientTest::parseOptions(argc, argv);
    if (ret != NOERROR) {
        return ret;
    }

    return NOERROR;
}

//=============================================================================
ARXDVPTest::ARXDVPTest()
{
    mGraph = NULL;
    mUseDVP = true;
}

ARXDVPTest::~ARXDVPTest()
{
    mCamRend[1]->setDVPGraph(NULL);
    delete mGraph;
}

arxstatus_t ARXDVPTest::init()
{
    arxstatus_t ret = ARXClientTest::init();
    if (ret != NOERROR) {
        return ret;
    }

    mGraph = new DVPTestGraph(mSecCamWidth, mSecCamHeight);
    if (mGraph == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed allocating a dvp test graph!");
        return NOMEMORY;
    }

    ret = mGraph->init(mCamWidth, mSecCamHeight, mArx);
    if (ret != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed initializing dvp test graph!");
        return ret;
    }

    mCamRend[1]->setDVPGraph(mGraph);

    return NOERROR;
}

}
