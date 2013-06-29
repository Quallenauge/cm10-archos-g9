/*
 *  Copyright (C) 2009-2011 Texas Instruments, Inc.
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

#include <dvp/VisionEngine.h>

/*! \brief The list of supported Graphs in TestVisionEngine.
 * \ingroup group_tests
 */
typedef enum _vision_engine_tests_e
{
    TI_GRAPH_TYPE_EDGE,     //0
    TI_GRAPH_TYPE_RVM,      //1
    TI_GRAPH_TYPE_X,        //2
    TI_GRAPH_TYPE_TEST1,    //3
    TI_GRAPH_TYPE_TEST2,    //4
    TI_GRAPH_TYPE_TEST3,    //5
    TI_GRAPH_TYPE_TEST4,    //6
    TI_GRAPH_TYPE_TEST5,    //7  Common kernels
    TI_GRAPH_TYPE_TEST6,    //8  VRUN
    TI_GRAPH_TYPE_TEST7,    //9  Same as TEST5, with A9 affinity
    TI_GRAPH_TYPE_TEST8,    //10 Tesla Only (histogram)
    TI_GRAPH_TYPE_TEST9,    //11 NV12 Camera Input
    TI_GRAPH_TYPE_TEST10,   //12 UYVY Input, Half and Quarter Scale UYVY Output
    TI_GRAPH_TYPE_TEST11,   //13 UYVY Capture, IYUV Convert, Rotate90 and Rotate270
    TI_GRAPH_TYPE_DISPARITY,//14 NV12 Stereo Capture, SAD8 To Produce a final image.
    TI_GRAPH_TYPE_TISMO,    //15 NV12 Stereo Capture, TISMO To Produce a disparity map.
    TI_GRAPH_TYPE_IMGLIB,   //16 Various Imglib functions.
    TI_GRAPH_TYPE_LDC,      //17 Simcop LDC functions.
    TI_GRAPH_TYPE_AR,       //18 AR test functions.
    TI_GRAPH_TYPE_BOUNDARY, //19 Border/perimeter test functions.
    TI_GRAPH_TYPE_HARRIS,   //20 Harris Corner test function.
    TI_GRAPH_TYPE_ORBHARRIS,//21 ORB+HarrisCorner test function.
    TI_GRAPH_TYPE_TESLA,    //22 Tesla only.
    TI_GRAPH_TYPE_IMGLIB_TESLA, //23 Imglib on Tesla
    TI_GRAPH_TYPE_DSPLIB_TESLA, //24 Dsplib on Tesla
    TI_GRAPH_TYPE_VLIB_CPU, //25 Dsplib on Tesla
    TI_GRAPH_TYPE_IMGLIB_CPU, //26 Dsplib on Tesla
    TI_GRAPH_TYPE_TISMOV02, //27 NV12 Stereo Capture, TISMOv02 To Produce a disparity map.    
    VENDOR_GRAPH_TYPE_BASE = 0x1000,    // Put vendor specific graph tests after here

} TI_GraphType_e;

/*! \brief The Kernel Regression Test Class.
 * \ingroup group_tests
 */
class TestVisionEngine : public VisionEngine
{
protected:
    TI_GraphType_e      m_graphtype;     /*!< Which graph is being run */
    DVP_Core_e          m_testcore;      /*!< Which core to run the graph on */
    DVP_Image_t         m_dbgImage;
public:
    TestVisionEngine(uint32_t width, uint32_t height, uint32_t fps, fourcc_t color, char *camName, uint32_t numFrames, int32_t graph, DVP_Core_e affinity);
    ~TestVisionEngine();

    // Implemenation over BaseClass
    status_e GraphSetup();
    status_e GraphUpdate(VisionCamFrame *cameraFrame);
    void GraphSectionComplete(DVP_KernelGraph_t *graph, uint32_t sectionIndex, uint32_t numNodesExecuted);
    virtual status_e PostProcessImage(VisionCamFrame* pFrame, uint32_t numSections);

    // Different Graph Setups
    status_e EdgeGraphSetup();
    status_e Test_MorphGraphSetup();
    status_e Test_NonmaxImgconvGraphSetup();
    status_e Test_CannyGraphSetup();
    status_e Test_MiscGraphSetup();
    status_e Test_VrunGraphSetup();
    status_e Test_VrunGraphSetup2();
    status_e Test_CommonGraphSetup();
    status_e Test_HistGraphSetup();
    status_e Test_NV12GraphSetup();
    status_e Test_UYVYScalingGraph();
    status_e Test_Rotate();
    status_e Test_Disparity();
    status_e Test_Tismo();
    status_e Test_Tismov02();
    status_e Test_Imglib();
    status_e Test_Ldc();
    status_e Test_AR();
    status_e Test_Boundary();
    status_e Test_ORBnHarris();
    status_e Test_HarrisCorner();
    status_e Test_TeslaGraphSetup();
    status_e Test_RVM();
    status_e Test_DSPLIB();
};

