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
#include <ARXClientTest.h>
#include <sosal/options.h>
#include <binder/ProcessState.h>

using namespace android;
using namespace tiarx;

enum {
    ARX_MAIN_TEST = 0,
    ARX_CAMPOSE_TEST,
    ARX_DVP_TEST,
};

int testCase = ARX_MAIN_TEST;
option_t opts[] = {
    {OPTION_TYPE_INT, &testCase, sizeof(int), "-tc", "--test-case", "The test case to run"},
};
uint32_t numOpts = dimof(opts);

int main(int argc, char *argv[])
{
    ARX_PRINT(ARX_ZONE_ALWAYS, "Starting ARAccelerator Test\n");

    ProcessState::self()->startThreadPool();

    // process the command line
    option_process(argc, argv, opts, numOpts);

    ARXClientTest *arxTest = NULL;
    switch (testCase) {
        case ARX_CAMPOSE_TEST:
            arxTest = new ARXCamPoseTest();
            break;
        case ARX_DVP_TEST:
            arxTest = new ARXDVPTest();
            break;
        case ARX_MAIN_TEST:
        default:
            arxTest = new ARXMainClientTest();
            break;
    }

    //This will delete the arx pointer when it goes out of scope
    AutoDestroy<ARXClientTest> ad(arxTest);

    arxstatus_t ret = arxTest->parseOptions(argc, argv);
    if (ret != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to parse options!\n");
        return ret;
    }

    ret = arxTest->init();
    if (ret != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to initialize test!\n");
        return ret;
    }

    ret = arxTest->start();
    if (ret != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to start test!\n");
        return ret;
    }

    ARX_PRINT(ARX_ZONE_ALWAYS, "Waiting for test to complete...\n");
    arxTest->waitToComplete();
    ARX_PRINT(ARX_ZONE_ALWAYS, "Finished.\n");

    return 0;
}
