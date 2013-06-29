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

#include <arx/ARAccelerator.h>
#include <utils/RefBase.h>
#include <sosal/event.h>

namespace tiarx {

class ARXClientTest;
class ARXImgRenderer;
class TestPropListener;
class FDRenderer;

template <typename T>
class AutoDestroy
{
public:
    AutoDestroy(T *obj)
    {
        mObj = obj;
    }

    ~AutoDestroy()
    {
        delete mObj;
    }

private:
    T *mObj;
};


class ARXClientTest
{
public:

    ARXClientTest();
    virtual ~ARXClientTest();

    virtual arxstatus_t init();
    virtual arxstatus_t start();
    virtual arxstatus_t parseOptions(int argc, char *argv[]);
    void waitToComplete();

protected:
    virtual arxstatus_t loadEngine();

    uint32_t mCamWidth;
    uint32_t mCamHeight;
    uint32_t mSecCamWidth;
    uint32_t mSecCamHeight;
    int mCamId;
    int mCamFPS;
    int mCamMirror;
    int mSecCamMirror;
    bool mEnableSec;
    int mTimeToRun;
    int mComputeBufOut;

    ARAccelerator *mArx;
    TestPropListener *mPropL;
    ARXImgRenderer *mCamRend[2];

    bool mUseDVP;
    event_t mDoneEvt;

};

//=============================================================================
class ARXMainClientTest : public ARXClientTest
{
public:
    ARXMainClientTest();
    ~ARXMainClientTest();

    arxstatus_t init();
    arxstatus_t parseOptions(int argc, char *argv[]);

private:
    ARXImgRenderer *mStreamRend[4];
    FDRenderer *mFdRend;
    FDRenderer *mFPRend;

    bool_e mEnableFD;
    bool_e mEnableFDSync;
    bool_e mEnableHarris;
    bool_e mEnableGrad;
    bool_e mEnableSobel;
    bool_e mEnableFP;
    bool_e mEnableFPFilter;
    bool_e mLogFP;
    int32_t mConfThresh;
    int32_t mMinMove;
    int32_t mMaxMove;
};

//=============================================================================
class CamPoseListener;
class ARXCamPoseTest : public ARXClientTest
{
public:
    ARXCamPoseTest();
    ~ARXCamPoseTest();

    arxstatus_t init();
    arxstatus_t parseOptions(int argc, char *argv[]);

protected:
    arxstatus_t loadEngine();

private:
    CamPoseListener *mCamPoseL;
};

//=============================================================================
class DVPTestGraph;
class ARXDVPTest : public ARXClientTest
{
public:
    ARXDVPTest();
    ~ARXDVPTest();

    arxstatus_t init();

private:
    DVPTestGraph *mGraph;
};


}
