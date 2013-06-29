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

#if defined(DVP_USE_VLIB)
#include <vlib/dvp_kl_vlib.h>
#endif

#if defined(DVP_USE_IMGLIB)
#include <imglib/dvp_kl_imglib.h>
#endif

#if defined(DVP_USE_RVM)
#include <rvm/dvp_kl_rvm.h>
#endif

#if defined(DVP_USE_TISMO)
#include <tismo/dvp_kl_tismo.h>
#endif

#if defined(DVP_USE_TISMOV02)
#include <tismov02/dvp_kl_tismov02.h>
#endif

#if defined(DVP_USE_VRUN)
#include <vrun/dvp_kl_vrun.h>
#endif

#if defined(DVP_USE_ORB)
#include <orb/dvp_kl_orb.h>
#include <orb/dvp_orbPattern.h>
#endif

#if defined(DVP_USE_DSPLIB)
#include <dsplib/dvp_kl_dsplib.h>
#define TEST_DSPLIB
#include <dsplib/Test_DSPLIB.h>
#endif

#if defined(DVP_USE_YUV)
#include <yuv/dvp_kl_yuv.h>
#endif

#include <TestVisionEngine.h>

#if (defined(WIN32) && !defined(CYGWIN)) || defined(__QNX__) || defined(SYSBIOS)
#ifndef M_PI
#define M_PI 3.14159265358979323846  /**< Pi */
#endif
#endif

TestVisionEngine::TestVisionEngine(uint32_t width, uint32_t height, uint32_t fps, fourcc_t color, char *camName, uint32_t numFrames, int32_t graph, DVP_Core_e affinity)
    : VisionEngine(width, height, fps, color, camName, numFrames)
{
    m_graphtype = (TI_GraphType_e)graph;
    m_display_enabled = false_e;
    m_imgdbg_enabled = true_e;
    m_testcore = affinity;

#if defined(DVP_USE_IPC)
    DVP_PRINT(DVP_ZONE_ENGINE, "Using IPC!\n");
#endif

#if defined(DVP_USE_RCM)
    DVP_PRINT(DVP_ZONE_ENGINE, "Using RCM!\n");
#endif

#if defined(DVP_USE_OMAPRPC)
    DVP_PRINT(DVP_ZONE_ENGINE, "Using OMAPRPC!\n");
#endif

#if defined(DVP_USE_TILER)
    DVP_PRINT(DVP_ZONE_ENGINE, "Using TILER!\n");
#endif

#if defined(DVP_USE_ION)
    DVP_PRINT(DVP_ZONE_ENGINE, "Using ION!\n");
#endif

#if defined(DVP_USE_GRALLOC)
    DVP_PRINT(DVP_ZONE_ENGINE, "Using GRALLOC!\n");
#endif

#if defined(DVP_USE_BO)
    DVP_PRINT(DVP_ZONE_ENGINE, "Using OMAP BO!\n");
#endif

    DVP_PRINT(DVP_ZONE_ENGINE, "Using path to im files: %s\n", m_imgdbg_path);
}

TestVisionEngine::~TestVisionEngine()
{
    // do nothing
}

status_e TestVisionEngine::PostProcessImage(VisionCamFrame* pFrame, uint32_t numSections)
{
    pFrame = pFrame;
    numSections = numSections;
    return STATUS_SUCCESS;
}

status_e TestVisionEngine::GraphSetup()
{
    status_e status = STATUS_INVALID_PARAMETER;
    m_hDVP = DVP_KernelGraph_Init();
    DVP_PRINT(DVP_ZONE_ENGINE, "DVP Handle = %p, about to setup graph %d\n", (void*)m_hDVP, m_graphtype);
    if (m_hDVP)
    {
        AllocateGraphs(1);

        switch (m_graphtype)
        {
            case TI_GRAPH_TYPE_EDGE:
                status = EdgeGraphSetup();
                break;
            case TI_GRAPH_TYPE_RVM:
                status = Test_RVM();
                break;
            case TI_GRAPH_TYPE_TEST1:
                status = Test_MorphGraphSetup();
                break;
            case TI_GRAPH_TYPE_TEST2:
                status = Test_NonmaxImgconvGraphSetup();
                break;
            case TI_GRAPH_TYPE_TEST3:
                status = Test_CannyGraphSetup();
                break;
            case TI_GRAPH_TYPE_TEST4:
                status = Test_MiscGraphSetup();
                break;
            case TI_GRAPH_TYPE_TEST5:
                status = Test_CommonGraphSetup();
                break;
            case TI_GRAPH_TYPE_TEST6:
                status = Test_VrunGraphSetup();
                break;
            case TI_GRAPH_TYPE_TEST7:
                status = Test_VrunGraphSetup2();
                break;
            case TI_GRAPH_TYPE_TEST8:
                status = Test_HistGraphSetup();
                break;
            case TI_GRAPH_TYPE_TEST9:
                status = Test_NV12GraphSetup();
                break;
            case TI_GRAPH_TYPE_TEST10:
                status = Test_UYVYScalingGraph();
                break;
            case TI_GRAPH_TYPE_TEST11:
                status = Test_Rotate();
                break;
            case TI_GRAPH_TYPE_DISPARITY:
                status = Test_Disparity();
                break;
            case TI_GRAPH_TYPE_TISMO:
                status = Test_Tismo();
                break;
            case TI_GRAPH_TYPE_TISMOV02:
                status = Test_Tismov02();
                break;
            case TI_GRAPH_TYPE_IMGLIB:
                status = Test_Imglib();
                break;
            case TI_GRAPH_TYPE_LDC:
                status = Test_Ldc();
                break;
            case TI_GRAPH_TYPE_AR:
                status = Test_AR();
                break;
            case TI_GRAPH_TYPE_BOUNDARY:
                status = Test_Boundary();
                break;
            case TI_GRAPH_TYPE_HARRIS:
                status = Test_HarrisCorner();
                break;
            case TI_GRAPH_TYPE_ORBHARRIS:
                status = Test_ORBnHarris();
                break;
            case TI_GRAPH_TYPE_TESLA:
                status = Test_TeslaGraphSetup();
                break;
            case TI_GRAPH_TYPE_IMGLIB_TESLA:
                m_testcore = DVP_CORE_DSP;
                status = Test_Imglib();
                break;
            case TI_GRAPH_TYPE_DSPLIB_TESLA:
                m_testcore = DVP_CORE_DSP;
                status = Test_DSPLIB();
                break;
            case TI_GRAPH_TYPE_VLIB_CPU:
                m_testcore = DVP_CORE_CPU;
                status = Test_TeslaGraphSetup();
                break;
            case TI_GRAPH_TYPE_IMGLIB_CPU:
                m_testcore = DVP_CORE_CPU;
                status = Test_Imglib();
                break;

            /* VENDOR Custom Test Graphs can be added here */


            /* End of VENDOR Custom Test Graphs */

            default:
                DVP_PRINT(DVP_ZONE_ERROR, "Invalid Graph Type\n");
                break;
        }
    }
    DVP_PRINT(DVP_ZONE_ENGINE, "GraphSetup status is %d\n", status);
    return status;
}

status_e TestVisionEngine::Test_DSPLIB()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_DSPLIB)
    DVP_U32 elemSize = sizeof(DVP_U16);
    DVP_U32 numElems = 128;
    DVP_U08 noNodes = 0;

    DVP_PRINT(DVP_ZONE_ERROR, "Test_DSPLIB: begins\n");


    if (m_hDVP)
    {
        if ( AllocateImageStructs(3))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2],  3, 3, FOURCC_Y800);
            if(!DVP_Image_Alloc(m_hDVP, &m_images[0], DVP_MTYPE_DEFAULT) ||
               !DVP_Image_Alloc(m_hDVP, &m_images[1], DVP_MTYPE_DEFAULT) ||
               !DVP_Image_Alloc(m_hDVP, &m_images[2], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        if ( AllocateBufferStructs(123))
        {
            DVP_Buffer_Init(&m_buffers[0], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[1], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[2], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[3], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[4], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[5], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[6], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[7], elemSize, (numElems)>>1);
            DVP_Buffer_Init(&m_buffers[8], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[9], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[10], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[11], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[12], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[13], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[14], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[15], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[16], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[17], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[18], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[19], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[20], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[21], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[22], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[23], elemSize,sizeof(FFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[24], elemSize, sizeof(FFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[25], elemSize,sizeof(FFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[26], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[27], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[28], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[29], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[30], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[31], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[32], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[33], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[34], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[35], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[36], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[37], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[38], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[39], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[40], elemSize, numElems);

            DVP_Buffer_Init(&m_buffers[41], elemSize, sizeof(FIR_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[42], elemSize, sizeof(FIR_h)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[43], elemSize, sizeof(FIR_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[44], elemSize, sizeof(FIR_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[45], elemSize, sizeof(FIR_h)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[46], elemSize, sizeof(FIR_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[47], elemSize, sizeof(FIR_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[48], elemSize, sizeof(FIR_h)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[49], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[50], elemSize, sizeof(FIR_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[51], elemSize, sizeof(FIR_h)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[52], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[53], elemSize, sizeof(FIR_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[54], elemSize, sizeof(FIR_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[55], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[56], elemSize, sizeof(FIR_Rx)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[57], elemSize, sizeof(FIR_Rh)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[58], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[59], elemSize, sizeof(FIR_Rx)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[60], elemSize, sizeof(FIR_Rh)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[61], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[62], elemSize, sizeof(FIR_Rx)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[63], elemSize, sizeof(FIR_Rh)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[64], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[65], elemSize, sizeof(FIR_xS)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[66], elemSize, sizeof(FIR_hs)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[67], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[68], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[69], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[70], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[71], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[72], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[73], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[74], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[75], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[76], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[77], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[78], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[79], elemSize, sizeof(IFFT_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[80], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[81], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[82], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[83], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[84], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[85], elemSize, sizeof(x_IIR)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[86], elemSize, sizeof(k_IIR)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[87], elemSize, sizeof(b_c_IIR)/sizeof(DVP_U16));;
            DVP_Buffer_Init(&m_buffers[88], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[89], elemSize, sizeof(ptr_Coefs)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[90], elemSize, (N_IIRSS + PAD_IIRSS));
            DVP_Buffer_Init(&m_buffers[91], elemSize, 256);
            DVP_Buffer_Init(&m_buffers[92], elemSize, 256);
            DVP_Buffer_Init(&m_buffers[93], elemSize, 256);
            DVP_Buffer_Init(&m_buffers[94], elemSize, 256);
            DVP_Buffer_Init(&m_buffers[95], elemSize, 256);
            DVP_Buffer_Init(&m_buffers[96], elemSize, 256);
            DVP_Buffer_Init(&m_buffers[97], elemSize, 256);
            DVP_Buffer_Init(&m_buffers[98], elemSize, 256);
            DVP_Buffer_Init(&m_buffers[99], elemSize, sizeof(buffer_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[100], elemSize, sizeof(buffer_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[101], elemSize, sizeof(buffer_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[102], elemSize, 256*9);
            DVP_Buffer_Init(&m_buffers[103], elemSize, 9);
            DVP_Buffer_Init(&m_buffers[104], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[105], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[106], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[107], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[108], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[109], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[110], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[111], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[112], elemSize, sizeof(FIR_x)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[113], elemSize, sizeof(a_W_VEC)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[114], elemSize, sizeof(a_W_VEC)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[115], elemSize, sizeof(a_W_VEC)/sizeof(DVP_U16));
            DVP_Buffer_Init(&m_buffers[116], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[117], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[118], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[119], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[120], elemSize, numElems);
            DVP_Buffer_Init(&m_buffers[121], elemSize, 256);
            DVP_Buffer_Init(&m_buffers[122], elemSize, 256);

            DVP_PRINT(DVP_ZONE_ERROR, "Test_DSPLIB: End of Buffer Init\n");

            if ( !DVP_Buffer_Alloc(m_hDVP, &m_buffers[0], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[1], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[2], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[3], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[4], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[5], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[6], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[7], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[8], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[9], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[10], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[11], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[12], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[13], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[14], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[15], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[16], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[17], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[18], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[19], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[20], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[21], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[22], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[23], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[24], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[25], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[26], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[27], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[28], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[29], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[30], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[31], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[32], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[33], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[34], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[35], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[36], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[37], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[38], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[39], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[40], DVP_MTYPE_DEFAULT) ||

                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[41], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[42], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[43], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[44], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[45], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[46], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[47], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[48], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[49], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[50], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[51], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[52], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[53], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[54], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[55], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[56], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[57], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[58], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[59], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[60], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[61], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[62], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[63], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[64], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[65], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[66], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[67], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[68], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[69], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[70], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[71], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[72], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[73], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[74], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[75], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[76], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[77], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[78], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[79], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[80], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[81], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[82], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[83], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[84], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[85], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[86], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[87], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[88], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[89], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[90], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[91], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[92], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[93], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[94], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[95], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[96], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[97], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[98], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[99], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[100], DVP_MTYPE_DEFAULT)||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[101], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[102], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[103], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[104], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[105], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[106], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[107], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[108], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[109], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[110], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[111], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[112], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[113], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[114], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[115], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[116], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[117], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[118], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[119], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[120], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[121], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[122], DVP_MTYPE_DEFAULT))
            {
                return STATUS_NOT_ENOUGH_MEMORY;
            }
                DVP_PRINT(DVP_ZONE_ERROR, "Test_DSPLIB: End of Buffer Alloc\n");

        }
#ifdef TEST_DSPLIB
        memcpy(m_buffers[0].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[1].pData,buffer_x,sizeof(buffer_y));
        memcpy(m_buffers[3].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[4].pData,buffer_x,sizeof(buffer_y));
        memcpy(m_buffers[8].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[9].pData,buffer_x,sizeof(buffer_y));
        memcpy(m_buffers[11].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[13].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[15].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[17].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[18].pData,buffer_x,sizeof(buffer_y));
        memcpy(m_buffers[20].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[21].pData,buffer_x,sizeof(buffer_y));
        memcpy(m_buffers[23].pData,FFT_x,sizeof(FFT_x));
        memcpy(m_buffers[24].pData,FFT_x,sizeof(FFT_x));
        memcpy(m_buffers[26].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[27].pData,buffer_x,sizeof(buffer_y));
        memcpy(m_buffers[29].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[30].pData,buffer_x,sizeof(buffer_y));
        memcpy(m_buffers[32].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[33].pData,buffer_x,sizeof(buffer_y));
        memcpy(m_buffers[35].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[36].pData,buffer_x,sizeof(buffer_y));
        memcpy(m_buffers[38].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[39].pData,buffer_x,sizeof(buffer_y));

        memcpy(m_buffers[41].pData,FIR_x,sizeof(FIR_x));
        memcpy(m_buffers[42].pData,FIR_h,sizeof(FIR_h));
        memcpy(m_buffers[44].pData,FIR_x,sizeof(FIR_x));
        memcpy(m_buffers[45].pData,FIR_h,sizeof(FIR_h));
        memcpy(m_buffers[47].pData,FIR_x,sizeof(FIR_x));
        memcpy(m_buffers[48].pData,FIR_h,sizeof(FIR_h));
        memcpy(m_buffers[50].pData,FIR_x,sizeof(FIR_x));
        memcpy(m_buffers[51].pData,FIR_h,sizeof(FIR_h));
        memcpy(m_buffers[53].pData,FIR_x,sizeof(FIR_x));
        memcpy(m_buffers[54].pData,FIR_x,sizeof(FIR_x));

        memcpy(m_buffers[56].pData,FIR_Rx,sizeof(FIR_Rx));
        memcpy(m_buffers[57].pData,FIR_Rh,sizeof(FIR_Rh));
        memcpy(m_buffers[59].pData,FIR_Rx,sizeof(FIR_Rx));
        memcpy(m_buffers[60].pData,FIR_Rh,sizeof(FIR_Rh));
        memcpy(m_buffers[62].pData,FIR_Rx,sizeof(FIR_Rx));
        memcpy(m_buffers[63].pData,FIR_Rh,sizeof(FIR_Rh));

        memcpy(m_buffers[65].pData,FIR_xS,sizeof(FIR_xS));
        memcpy(m_buffers[66].pData,FIR_hs,sizeof(FIR_hs));

        memcpy(m_buffers[68].pData,IFFT_x,sizeof(IFFT_x));
        memcpy(m_buffers[69].pData,IFFT_x,sizeof(IFFT_x));
        memcpy(m_buffers[71].pData,IFFT_x,sizeof(IFFT_x));
        memcpy(m_buffers[72].pData,IFFT_x,sizeof(IFFT_x));
        memcpy(m_buffers[74].pData,IFFT_x,sizeof(IFFT_x));
        memcpy(m_buffers[75].pData,IFFT_x,sizeof(IFFT_x));
        memcpy(m_buffers[77].pData,IFFT_x,sizeof(IFFT_x));
        memcpy(m_buffers[78].pData,IFFT_x,sizeof(IFFT_x));

        memcpy(m_buffers[85].pData,x_IIR,sizeof(x_IIR));
        memcpy(m_buffers[86].pData,k_IIR,sizeof(k_IIR));
        memcpy(m_buffers[87].pData,b_c_IIR,sizeof(b_c_IIR));
        memcpy(m_buffers[89].pData,ptr_Coefs,sizeof(ptr_Coefs));

        memcpy(m_buffers[91].pData,MAT_x,sizeof(MAT_x));
        memcpy(m_buffers[92].pData,MAT_y,sizeof(MAT_y));
        memcpy(m_buffers[94].pData,MAT_x,sizeof(MAT_x));
        memcpy(m_buffers[95].pData,MAT_y,sizeof(MAT_y));
        memcpy(m_buffers[97].pData,MAT_x,sizeof(MAT_x));
        memcpy(m_buffers[99].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[100].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[101].pData,buffer_x,sizeof(buffer_x));
        memcpy(m_buffers[102].pData,GSP0_TABLE,sizeof(GSP0_TABLE));
        memcpy(m_buffers[103].pData,errCoefs,sizeof(errCoefs));
        memcpy(m_buffers[112].pData,FIR_x,sizeof(FIR_x));
        memcpy(m_buffers[113].pData,a_W_VEC,sizeof(a_W_VEC));
        memcpy(m_buffers[114].pData,a_W_VEC,sizeof(a_W_VEC));

        memcpy(m_buffers[121].pData,MAT_x,sizeof(MAT_x)/sizeof(DVP_U16));


        DVP_PRINT(DVP_ZONE_ERROR, "Test_DSPLIB: End of Memcopy\n");

#endif

        if (AllocateNodes(17) && AllocateSections(&m_graphs[0], 1))
        {
#if defined(DVP_USE_IMGLIB)
            m_pNodes[noNodes].header.kernel = DVP_KN_IMG_CONV_3x3;
            dvp_knode_to(&m_pNodes[noNodes], DVP_ImageConvolution_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[noNodes], DVP_ImageConvolution_t)->output = m_images[1];
            dvp_knode_to(&m_pNodes[noNodes], DVP_ImageConvolution_t)->mask = m_images[2];
            dvp_knode_to(&m_pNodes[noNodes], DVP_ImageConvolution_t)->shiftMask = 6;
            noNodes++;
#endif
#if 0
            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_ADD16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[0];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input1 = m_buffers[1];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[2];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_ADD32;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[3];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input1 = m_buffers[4];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[5];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_AUTOCORR16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[6];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[7];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_BITEXP32;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[8];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_BLKESWAP16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[9];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[10];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_BLKESWAP32;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[11];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[12];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_BLKESWAP64;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[13];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[14];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_BLKMOVE;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[15];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[16];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_DOTPRODSQR;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[17];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input1 = m_buffers[18];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[19];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1 = 2;//Needs attn
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_DOTPROD;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[20];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input1 = m_buffers[21];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[22];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_16x16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[23];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[24];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[25];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_16x16R;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[26];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[27];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[28];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1 = 2;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val2 = 2;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val3 = 8;
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_16x16_IMRE;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[29];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[30];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[31];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_16x32;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[32];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[33];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[34];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_32x32;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[35];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[36];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[37];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FFT_32x32s;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[38];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[39];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[40];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_CPLX;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[41];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[42];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[43];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1  = sizeof(FIR_h)>>1;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val2  = sizeof(FIR_x)>>1;
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_HM4x4;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[44];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[45];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[46];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1  = sizeof(FIR_h)>>1;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val2  = sizeof(FIR_x)>>1;
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_GEN;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[47];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[48];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[49];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_HM17_RA8x8;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[50];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[51];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[52];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_LMS2;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[53];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[54];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[55];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1  = 0x1000;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val2  = 0x0; // Return value
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_R4;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[56];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[57];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[58];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_R8;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[59];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[60];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[61];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_HM16_RM8A8x8;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[62];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[63];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[64];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_FIR_SYM;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[65];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[66];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[67];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1  = 0;
            noNodes++; //25
#endif

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_IFFT_16x16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[68];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[69];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[70];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1  = (sizeof(IFFT_x)/sizeof(DVP_U16))>>1; // Size of coefficient array = 2*nx
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_IFFT_16x16_IMRE;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[71];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[72];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[73];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1  = (sizeof(IFFT_x)/sizeof(DVP_U16))>>1;
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_IFFT_16x32;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[74];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[75];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[76];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1  = (sizeof(IFFT_x)/sizeof(DVP_U16))>>1;
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_IFFT_32x32;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[77];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[78];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[79];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1  =  (sizeof(IFFT_x)/sizeof(DVP_U16))>>1;
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_IIR;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[80];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input1 = m_buffers[81];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[82];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[83];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output1 = m_buffers[84];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1   = numElems;
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_IIR_LAT;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[85];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input1 = m_buffers[86];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[87];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[88];
            noNodes++;//3  //31

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_IIR_SS;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1 = 0;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[89];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input1 = m_buffers[90];
            noNodes++;


            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_MUL;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->input0 = m_buffers[91];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->input1 = m_buffers[92];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->output0 = m_buffers[93];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->r1 = 16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->c1 = 16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->r2 = 16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->c2 = 16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->qs = 1;
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_MUL_CPLX;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->input0 = m_buffers[94];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->input1 = m_buffers[95];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->output0 = m_buffers[96];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->r1 = 16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->c1 = 16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->r2 = 16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->c2 = 16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSP_MatMul)->qs = 1;
            noNodes++;


            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_W_VEC;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[113];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input1 = m_buffers[114];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[115];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1 = 16384;
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_MAXVAL;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[99];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_MINVAL;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[100];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_MAXIDX;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[101];
            noNodes++;


            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_MINERROR;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[102];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input1 = m_buffers[103];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_MUL32;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[104];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input1 = m_buffers[105];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[106];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1 = numElems;
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_NEG32;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[107];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[108];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1 = numElems;
            noNodes++;

#if 0
            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_RECIP16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[109];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->twoway = m_buffers[110];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[111];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1 = numElems;
            noNodes++;


            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_VECSUMSQ;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[112];
            noNodes++;

            m_pNodes[noNodes].header.kernel = DVP_KN_DSP_MAT_TRANS;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->input0 = m_buffers[121];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->output0 = m_buffers[122];
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val1 = 16;
            dvp_knode_to(&m_pNodes[noNodes], DVP_DSPFunc)->val2 = 16;
            noNodes++;
#endif

            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }

    }


    if(m_testcore != DVP_CORE_MIN)
    {
            for(uint32_t testnode=0; testnode<m_numNodes; testnode++) {
                m_pNodes[testnode].header.affinity = m_testcore;
            }
    }
    status = CameraInit(this, m_images[0].color);

    DVP_Perf_Clear(&m_graphs[0].totalperf);
    DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
#endif

    return status;
}



status_e TestVisionEngine::EdgeGraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VLIB)
    if (m_fourcc != FOURCC_UYVY)
        return STATUS_INVALID_PARAMETER;

    if (m_hDVP)
    {
        if (AllocateImageStructs(7))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[6], m_width, m_height, FOURCC_Y800);
            m_camMin = m_dispMin = 0;
            m_camMax = m_dispMax = 4;
            m_camIdx = m_dispIdx = 0;
            if (m_display_enabled)
            {
                m_display = DVP_Display_Create(m_width, m_height, m_width, m_height, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, m_width, m_height, 0, 0, m_fourcc, 0, m_dispMax+1);
                DVP_Display_Alloc(m_display, &m_images[0]);
                DVP_Display_Alloc(m_display, &m_images[1]);
                DVP_Display_Alloc(m_display, &m_images[2]);
                DVP_Display_Alloc(m_display, &m_images[3]);
            }
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
            else {
                m_images[0].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
                m_images[1].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
                m_images[2].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
                m_images[3].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
            }
#endif

            if (!DVP_Image_Alloc(m_hDVP, &m_images[0],(DVP_MemType_e)m_images[0].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1],(DVP_MemType_e)m_images[1].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2],(DVP_MemType_e)m_images[2].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3],(DVP_MemType_e)m_images[3].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(3) && AllocateSections(&m_graphs[0], 1)) // these are in shared memory
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[4];

            m_pNodes[1].header.kernel = DVP_KN_SCHARR_8;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[4];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[5];

            m_pNodes[2].header.kernel = DVP_KN_ERODE_CROSS;
            dvp_knode_to(&m_pNodes[2], DVP_Morphology_t)->input = m_images[5];
            dvp_knode_to(&m_pNodes[2], DVP_Morphology_t)->output = m_images[6];

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(3))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[4], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[1], &m_images[5], m_imgdbg_path, "02_edge");
                ImageDebug_Init(&m_imgdbg[2], &m_images[6], m_imgdbg_path, "03_erode");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;
    }
    else
        status = STATUS_NO_RESOURCES;
#else
    status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

#if 0 // depreciated ... to be removed
status_e TestVisionEngine::SVGA_RVM_GraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_RVM)
    DVP_Bounds_t bound = {NULL, 0, 0};
    DVP_U32 width = (DVP_U32)(m_width / 2.5);
    DVP_U32 height = (DVP_U32)(m_height / 3.75);

    if (m_hDVP)
    {
        if (AllocateImageStructs(12))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], width, height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[2], width, height, FOURCC_YV24);
            DVP_Image_Init(&m_images[3], width, height, FOURCC_Y800);
            DVP_Image_Init(&m_images[4], width, height, FOURCC_RGBP);
            DVP_Image_Init(&m_images[5], width, height, FOURCC_Y800);
            DVP_Image_Init(&m_images[6], width, height, FOURCC_Y800);
            DVP_Image_Init(&m_images[7], width, height, FOURCC_Y800);
            DVP_Image_Init(&m_images[8], width, height, FOURCC_Y800);
            DVP_Image_Init(&m_images[9], width/2, height/2+m_height/8+m_height/64, FOURCC_Y800);
            DVP_Image_Init(&m_images[10],(width-4)/2, (height-3)/2, FOURCC_Y800);
            DVP_Image_Init(&m_images[11],(width-4)/2, (height-4)/2, FOURCC_Y800);
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[8], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[9], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[10], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[11], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(10) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_RVM_SUNEX_UYVY;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];

            m_pNodes[1].header.kernel = DVP_KN_UYVY_TO_YUV444p;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[2];

            m_pNodes[2].header.kernel = DVP_KN_XYXY_TO_Y800;
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->output = m_images[3];

            m_pNodes[3].header.kernel = DVP_KN_UYVY_TO_RGBp;
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->output = m_images[4];

            m_pNodes[4].header.kernel = DVP_KN_IIR_HORZ;
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->input = m_images[3];
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->output = m_images[6];
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->weight = 2000;
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->bounds[0] = bound;
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->bounds[1] = bound;
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->scratch = m_images[5];

            m_pNodes[5].header.kernel = DVP_KN_IIR_VERT;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->input = m_images[3];
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->output = m_images[7];
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->weight = 2000;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->bounds[0] = bound;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->bounds[1] = bound;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->scratch = m_images[5];

            m_pNodes[6].header.kernel = DVP_KN_SCHARR_8;
            dvp_knode_to(&m_pNodes[6], DVP_Transform_t)->input = m_images[3];
            dvp_knode_to(&m_pNodes[6], DVP_Transform_t)->output = m_images[8];

            if(m_testcore == DVP_CORE_DSP || DVP_CORE_CPU)
            {
                m_pNodes[7].header.kernel = DVP_KN_VLIB_IMAGE_PYRAMID_8;
                dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->input = m_images[3];
                dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->output = m_images[9];
            }
#if defined(DVP_USE_VRUN)
            else
            {
                m_pNodes[7].header.kernel = DVP_KN_VRUN_IMAGE_PYRAMID_8;
                dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->input = m_images[3];
                dvp_knode_to(&m_pNodes[7], DVP_Pyramid_t)->output = m_images[9];
            }
#endif

            if(m_testcore == DVP_CORE_DSP || DVP_CORE_CPU)
            {
                m_pNodes[8].header.kernel = DVP_KN_VLIB_GRADIENT_V5x5_PYRAMID_8;
                dvp_knode_to(&m_pNodes[8], DVP_Gradient_t)->input = m_images[3];
                dvp_knode_to(&m_pNodes[8], DVP_Gradient_t)->output = m_images[10];
                dvp_knode_to(&m_pNodes[8], DVP_Gradient_t)->scratch = m_images[5];
            }
#if defined(DVP_USE_VRUN)
            else
            {
                m_pNodes[8].header.kernel = DVP_KN_VRUN_GRADIENT_V5x5_PYRAMID_8;
                dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->input = m_images[3];
                dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->output = m_images[10];
            }
#endif

            if(m_testcore == DVP_CORE_DSP || DVP_CORE_CPU)
            {
                m_pNodes[9].header.kernel = DVP_KN_VLIB_GRADIENT_H5x5_PYRAMID_8;
                dvp_knode_to(&m_pNodes[9], DVP_Gradient_t)->input = m_images[3];
                dvp_knode_to(&m_pNodes[9], DVP_Gradient_t)->output = m_images[11];
                dvp_knode_to(&m_pNodes[9], DVP_Gradient_t)->scratch = m_images[5];
            }
#if defined(DVP_USE_VRUN)
            else
            {
                m_pNodes[9].header.kernel = DVP_KN_VRUN_GRADIENT_H5x5_PYRAMID_8;
                dvp_knode_to(&m_pNodes[9], DVP_Transform_t)->input = m_images[3];
                dvp_knode_to(&m_pNodes[9], DVP_Transform_t)->output = m_images[11];
            }
#endif

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        else
            status = STATUS_NO_RESOURCES;

        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(10))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[1], m_imgdbg_path, "00_rvm");
                ImageDebug_Init(&m_imgdbg[1], &m_images[2], m_imgdbg_path, "01_YUV444");
                ImageDebug_Init(&m_imgdbg[2], &m_images[3], m_imgdbg_path, "02_luma");
                ImageDebug_Init(&m_imgdbg[3], &m_images[4], m_imgdbg_path, "03_RGBpl");
                ImageDebug_Init(&m_imgdbg[4], &m_images[6], m_imgdbg_path, "04_IIRH");
                ImageDebug_Init(&m_imgdbg[5], &m_images[7], m_imgdbg_path, "05_IIRV");
                ImageDebug_Init(&m_imgdbg[6], &m_images[8], m_imgdbg_path, "06_edge");
                ImageDebug_Init(&m_imgdbg[7], &m_images[9], m_imgdbg_path, "07_imgpyramid");
                ImageDebug_Init(&m_imgdbg[8], &m_images[10], m_imgdbg_path, "08_gradv");
                ImageDebug_Init(&m_imgdbg[9], &m_images[11], m_imgdbg_path, "09_gradh");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}
#endif

status_e TestVisionEngine::Test_MiscGraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VLIB)
    DVP_Bounds_t bound = {NULL, 0, 0};
    DVP_MemType_e dispType = DVP_MTYPE_DEFAULT;

    if (m_hDVP)
    {
        if (AllocateImageStructs(11))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_YV24);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[6], m_width, m_height, FOURCC_RGBP);
            DVP_Image_Init(&m_images[7], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[8], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[9], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[10], m_width, m_height, FOURCC_IYUV);
            m_camIdx = m_dispIdx = 0;
            m_camMin = m_dispMin = 0;
            m_camMax = m_dispMax = 4;
            if (m_display_enabled)
            {
                DisplayCreate(m_width, m_height, m_width, m_height, m_width, m_height, 0, 0);
                for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++)
                    DisplayAllocate(&m_images[m_dispIdx]);
                dispType = (DVP_MemType_e)m_images[m_dispMin].memType;
            }
#if  defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
            else {
                dispType = DVP_MTYPE_MPUNONCACHED_2DTILED;
            }
#endif
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], dispType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], dispType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2], dispType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], dispType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], DVP_MTYPE_DEFAULT) ||
#if defined(DVP_USE_ION) || defined(DVP_USE_TILER) || defined(DVP_USE_BO)
                !DVP_Image_Alloc(m_hDVP, &m_images[5], DVP_MTYPE_MPUNONCACHED_2DTILED) ||
#else
                !DVP_Image_Alloc(m_hDVP, &m_images[5], DVP_MTYPE_DEFAULT) ||
#endif
                !DVP_Image_Alloc(m_hDVP, &m_images[6], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[8], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[9], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[10], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
            // set cache op hints if any of these buffers will not be used on
            // the local host for processing (by default the hints always
            // indicate to do the operations).

//            m_images[0].skipCacheOpFlush = DVP_TRUE;
//            m_images[5].skipCacheOpInval = DVP_TRUE;
//            m_images[6].skipCacheOpInval = DVP_TRUE;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(6) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_UYVY_TO_YUV444p;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[4];
            //m_pNodes[0].header.affinity = DVP_CORE_CPU;

            m_pNodes[1].header.kernel = DVP_KN_XYXY_TO_Y800;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[5];
            m_pNodes[1].header.affinity = DVP_CORE_DSP;
           // m_pNodes[1].header.affinity = DVP_CORE_CPU;

            m_pNodes[2].header.kernel = DVP_KN_UYVY_TO_RGBp;
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->output = m_images[6];
            //m_pNodes[2].header.affinity = DVP_CORE_CPU;

            m_pNodes[3].header.kernel = DVP_KN_IIR_HORZ;
            dvp_knode_to(&m_pNodes[3], DVP_IIR_t)->input = m_images[5];
            dvp_knode_to(&m_pNodes[3], DVP_IIR_t)->output = m_images[7];
            dvp_knode_to(&m_pNodes[3], DVP_IIR_t)->weight = 2000;
            dvp_knode_to(&m_pNodes[3], DVP_IIR_t)->bounds[0] = bound;
            dvp_knode_to(&m_pNodes[3], DVP_IIR_t)->bounds[1] = bound;
            dvp_knode_to(&m_pNodes[3], DVP_IIR_t)->scratch = m_images[8];
            //m_pNodes[3].header.affinity = DVP_CORE_CPU;

            m_pNodes[4].header.kernel = DVP_KN_IIR_VERT;
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->input = m_images[5];
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->output = m_images[9];
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->weight = 2000;
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->bounds[0] = bound;
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->bounds[1] = bound;
            dvp_knode_to(&m_pNodes[4], DVP_IIR_t)->scratch = m_images[8];
            //m_pNodes[4].header.affinity = DVP_CORE_CPU;

            m_pNodes[5].header.kernel = DVP_KN_UYVY_TO_YUV420p;
            dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->output = m_images[10];
            //m_pNodes[5].header.affinity = DVP_CORE_CPU;

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(6))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[4], m_imgdbg_path, "00_YUV444");
                ImageDebug_Init(&m_imgdbg[1], &m_images[5], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[6], m_imgdbg_path, "02_RGBpl");
                ImageDebug_Init(&m_imgdbg[3], &m_images[7], m_imgdbg_path, "03_IIRH");
                ImageDebug_Init(&m_imgdbg[4], &m_images[9], m_imgdbg_path, "04_IIRV");
                ImageDebug_Init(&m_imgdbg[5], &m_images[10], m_imgdbg_path, "05_YUV420");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_MorphGraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VLIB)
    DVP_S08 maskOnes[3][3] = {{1,1,1},{1,1,1},{1,1,1}};
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e smallType = DVP_MTYPE_MPUCACHED_VIRTUAL;

#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_MPUNONCACHED_2DTILED;
    opType = DVP_MTYPE_MPUNONCACHED_1DTILED;
    smallType = DVP_MTYPE_MPUNONCACHED_1DTILED;
#endif

    if (m_hDVP)
    {
        if (AllocateImageStructs(10))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[6], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[7], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[8], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[9], 3, 3, FOURCC_Y800);
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], camType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType) || // DEFAULT is 1DTILED when using TILER
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[8], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[9], smallType))       // Mask can be in virtual space since it is copied
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(8) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;      // Get Luma first
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];
            //m_pNodes[0].header.affinity = DVP_CORE_CPU;

            m_pNodes[1].header.kernel = DVP_KN_THRESHOLD;         // Then threshold for binary image functions
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[1];       // Using Luma from Luma Extract function
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[2];      // Thresholded output
            //m_pNodes[1].header.affinity = DVP_CORE_CPU;

            m_pNodes[2].header.kernel = DVP_KN_ERODE_CROSS;
            dvp_knode_to(&m_pNodes[2], DVP_Morphology_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[2], DVP_Morphology_t)->output = m_images[3];
            //m_pNodes[2].header.affinity = DVP_CORE_CPU;

            m_pNodes[3].header.kernel = DVP_KN_ERODE_SQUARE;
            dvp_knode_to(&m_pNodes[3], DVP_Morphology_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[3], DVP_Morphology_t)->output = m_images[4];
            //m_pNodes[3].header.affinity = DVP_CORE_CPU;

            m_pNodes[4].header.kernel = DVP_KN_ERODE_MASK;
            dvp_knode_to(&m_pNodes[4], DVP_Morphology_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[4], DVP_Morphology_t)->output = m_images[5];
            dvp_knode_to(&m_pNodes[4], DVP_Morphology_t)->mask = m_images[9];
            //m_pNodes[4].header.affinity = DVP_CORE_CPU;

            m_pNodes[5].header.kernel = DVP_KN_DILATE_CROSS;
            dvp_knode_to(&m_pNodes[5], DVP_Morphology_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[5], DVP_Morphology_t)->output = m_images[6];
            //m_pNodes[5].header.affinity = DVP_CORE_CPU;

            m_pNodes[6].header.kernel = DVP_KN_DILATE_SQUARE;
            dvp_knode_to(&m_pNodes[6], DVP_Morphology_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[6], DVP_Morphology_t)->output = m_images[7];
            //m_pNodes[6].header.affinity = DVP_CORE_CPU;

            m_pNodes[7].header.kernel = DVP_KN_DILATE_MASK;
            dvp_knode_to(&m_pNodes[7], DVP_Morphology_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[7], DVP_Morphology_t)->output = m_images[8];
            dvp_knode_to(&m_pNodes[7], DVP_Morphology_t)->mask = m_images[9];
            //m_pNodes[7].header.affinity = DVP_CORE_CPU;

            DVP_Image_Fill(&m_images[9], (DVP_S08 *)maskOnes, 3*3*sizeof(DVP_S08)); //  Setting the mask to be a square (all 1s)

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(9))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_bitimgThresh");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "03_bitimgErodeX");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "04_bitimgErodeSq");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "05_bitimgErodeMsk");
                ImageDebug_Init(&m_imgdbg[6], &m_images[6], m_imgdbg_path, "06_bitimgDilateX");
                ImageDebug_Init(&m_imgdbg[7], &m_images[7], m_imgdbg_path, "07_bitimgDilateSq");
                ImageDebug_Init(&m_imgdbg[8], &m_images[8], m_imgdbg_path, "08_bitimgDilateMsk");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_NonmaxImgconvGraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VLIB) && defined(DVP_USE_IMGLIB)
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e smallType = DVP_MTYPE_MPUCACHED_VIRTUAL;

#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_MPUNONCACHED_2DTILED;
    opType = DVP_MTYPE_MPUNONCACHED_1DTILED;
    smallType = DVP_MTYPE_MPUNONCACHED_1DTILED;
#endif

    DVP_S08 maskBlurNeg3x3[3][3] = {{-5, 20, 17} ,
                                    {19, -28, 13} ,
                                    {-22, 9, 33}};
    DVP_S08 maskBlur7x7[7][7]    = {{5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5}};
    if (m_hDVP)
    {
        if (AllocateImageStructs(12))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[6], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[7], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[8], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[9],  3, 3, FOURCC_Y800);
            DVP_Image_Init(&m_images[10], 5, 5, FOURCC_Y800);
            DVP_Image_Init(&m_images[11], 7, 7, FOURCC_Y800);
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], camType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[8], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[9],  smallType) ||       // Mask can be in virtual space since it is copied
                !DVP_Image_Alloc(m_hDVP, &m_images[10], smallType) ||       // Mask can be in virtual space since it is copied
                !DVP_Image_Alloc(m_hDVP, &m_images[11], smallType))         // Mask can be in virtual space since it is copied
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(8) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;      // Get Luma first
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];
            //m_pNodes[0].header.affinity = DVP_CORE_CPU;

            m_pNodes[1].header.kernel = DVP_KN_XSTRIDE_CONVERT;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[2];
            //m_pNodes[1].header.affinity = DVP_CORE_CPU;

            m_pNodes[2].header.kernel = DVP_KN_VLIB_NONMAXSUPPRESS_3x3_S16;
            dvp_knode_to(&m_pNodes[2], DVP_Threshold_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[2], DVP_Threshold_t)->output = m_images[3];
            dvp_knode_to(&m_pNodes[2], DVP_Threshold_t)->thresh = 100;
            if (dvp_knode_to(&m_pNodes[2], DVP_Threshold_t)->input.width > 992)
               m_pNodes[2].header.affinity = DVP_CORE_CPU;

            m_pNodes[3].header.kernel = DVP_KN_VLIB_NONMAXSUPPRESS_5x5_S16;
            dvp_knode_to(&m_pNodes[3], DVP_Threshold_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[3], DVP_Threshold_t)->output = m_images[4];
            dvp_knode_to(&m_pNodes[3], DVP_Threshold_t)->thresh = 100;
            if (dvp_knode_to(&m_pNodes[3], DVP_Threshold_t)->input.width > 992)
               m_pNodes[3].header.affinity = DVP_CORE_CPU;

            m_pNodes[4].header.kernel = DVP_KN_VLIB_NONMAXSUPPRESS_7x7_S16;
            dvp_knode_to(&m_pNodes[4], DVP_Threshold_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[4], DVP_Threshold_t)->output = m_images[5];
            dvp_knode_to(&m_pNodes[4], DVP_Threshold_t)->thresh = 100;
            if(dvp_knode_to(&m_pNodes[4], DVP_Threshold_t)->input.width > 768)
               m_pNodes[4].header.affinity = DVP_CORE_CPU;

            m_pNodes[5].header.kernel = DVP_KN_IMG_CONV_3x3;
            dvp_knode_to(&m_pNodes[5], DVP_ImageConvolution_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[5], DVP_ImageConvolution_t)->output = m_images[6];
            dvp_knode_to(&m_pNodes[5], DVP_ImageConvolution_t)->mask = m_images[9];
            dvp_knode_to(&m_pNodes[5], DVP_ImageConvolution_t)->shiftMask = 6;
            //m_pNodes[5].header.affinity = DVP_CORE_CPU;

            m_pNodes[6].header.kernel = DVP_KN_IMG_CONV_5x5;
            dvp_knode_to(&m_pNodes[6], DVP_ImageConvolution_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[6], DVP_ImageConvolution_t)->output = m_images[7];
            dvp_knode_to(&m_pNodes[6], DVP_ImageConvolution_t)->mask = m_images[10];
            dvp_knode_to(&m_pNodes[6], DVP_ImageConvolution_t)->shiftMask = 7;
            //m_pNodes[6].header.affinity = DVP_CORE_CPU;

            m_pNodes[7].header.kernel = DVP_KN_IMG_CONV_7x7;
            dvp_knode_to(&m_pNodes[7], DVP_ImageConvolution_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[7], DVP_ImageConvolution_t)->output = m_images[8];
            dvp_knode_to(&m_pNodes[7], DVP_ImageConvolution_t)->mask = m_images[11];
            dvp_knode_to(&m_pNodes[7], DVP_ImageConvolution_t)->shiftMask = 8;
            //m_pNodes[7].header.affinity = DVP_CORE_CPU;

            DVP_Image_Fill(&m_images[9], (DVP_S08*)maskBlurNeg3x3, 3*3*sizeof(DVP_S08));
            DVP_Image_Fill(&m_images[10], (DVP_S08*)maskBlur7x7,    5*5*sizeof(DVP_S08));
            DVP_Image_Fill(&m_images[11], (DVP_S08*)maskBlur7x7,    7*7*sizeof(DVP_S08));

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;

        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(9))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_16bitStride");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "03_nonma3");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "04_nonma5");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "05_nonma7");
                ImageDebug_Init(&m_imgdbg[6], &m_images[6], m_imgdbg_path, "06_imgconv3");
                ImageDebug_Init(&m_imgdbg[7], &m_images[7], m_imgdbg_path, "07_imgconv5");
                ImageDebug_Init(&m_imgdbg[8], &m_images[8], m_imgdbg_path, "08_imgconv7");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_CannyGraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VLIB) && defined(DVP_USE_IMGLIB)
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e smallType = DVP_MTYPE_MPUCACHED_VIRTUAL;

#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_MPUNONCACHED_2DTILED;
    opType = DVP_MTYPE_MPUNONCACHED_1DTILED;
    smallType = DVP_MTYPE_MPUNONCACHED_1DTILED;
#endif

    // 3x3 Gaussian kernel with SQ0.8 coefficients
    const DVP_S08 gaussian_7x7[49] =
    {
        0, 0,  0,  0,  0, 0, 0,
        0, 0,  0,  0,  0, 0, 0,
        0, 0, 16, 32, 16, 0, 0,
        0, 0, 32, 64, 32, 0, 0,
        0, 0, 16, 32, 16, 0, 0,
        0, 0,  0,  0,  0, 0, 0,
        0, 0,  0,  0,  0, 0, 0
    };

    if (m_hDVP)
    {
        if (AllocateImageStructs(12))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[3], 7, 7, FOURCC_Y800);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[6], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[7], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[8], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[9], m_width, m_height, FOURCC_RGBA);
            DVP_Image_Init(&m_images[10], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[11], m_width, m_height, FOURCC_Y800);
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], camType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], smallType)    ||  // Mask can be in virtual space since it is copied
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[8], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[9], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[10], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[11], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(8) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;      // Get Luma first
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];
            //m_pNodes[0].header.affinity = DVP_CORE_CPU;

            m_pNodes[1].header.kernel = DVP_KN_CANNY_IMAGE_SMOOTHING;
            dvp_knode_to(&m_pNodes[1], DVP_ImageConvolution_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[1], DVP_ImageConvolution_t)->output = m_images[2];
            dvp_knode_to(&m_pNodes[1], DVP_ImageConvolution_t)->mask = m_images[3];
            dvp_knode_to(&m_pNodes[1], DVP_ImageConvolution_t)->shiftMask = 8;
            //m_pNodes[1].header.affinity = DVP_CORE_CPU;

            m_pNodes[2].header.kernel = DVP_KN_CANNY_2D_GRADIENT;
            dvp_knode_to(&m_pNodes[2], DVP_Canny2dGradient_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[2], DVP_Canny2dGradient_t)->outGradX = m_images[4];
            dvp_knode_to(&m_pNodes[2], DVP_Canny2dGradient_t)->outGradY = m_images[5];
            dvp_knode_to(&m_pNodes[2], DVP_Canny2dGradient_t)->outMag = m_images[6];
            //m_pNodes[2].header.affinity = DVP_CORE_CPU;

            m_pNodes[3].header.kernel = DVP_KN_CANNY_NONMAX_SUPPRESSION;
            dvp_knode_to(&m_pNodes[3], DVP_CannyNonMaxSuppression_t)->inGradX = m_images[4];
            dvp_knode_to(&m_pNodes[3], DVP_CannyNonMaxSuppression_t)->inGradY = m_images[5];
            dvp_knode_to(&m_pNodes[3], DVP_CannyNonMaxSuppression_t)->inMag = m_images[6];
            dvp_knode_to(&m_pNodes[3], DVP_CannyNonMaxSuppression_t)->output = m_images[7];
           // m_pNodes[3].header.affinity = DVP_CORE_DSP;

            m_pNodes[4].header.kernel = DVP_KN_CANNY_HYST_THRESHHOLD;
            dvp_knode_to(&m_pNodes[4], DVP_CannyHystThresholding_t)->inMag = m_images[6];
            dvp_knode_to(&m_pNodes[4], DVP_CannyHystThresholding_t)->inEdgeMap = m_images[7];
            dvp_knode_to(&m_pNodes[4], DVP_CannyHystThresholding_t)->output = m_images[8];
            dvp_knode_to(&m_pNodes[4], DVP_CannyHystThresholding_t)->loThresh = 35;
            dvp_knode_to(&m_pNodes[4], DVP_CannyHystThresholding_t)->hiThresh = 122;
            //m_pNodes[4].header.affinity = DVP_CORE_CPU;

            m_pNodes[5].header.kernel = DVP_KN_INTEGRAL_IMAGE_8;
            dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->output = m_images[9];
            if(dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->input.width> 2049 ||dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->input.height> 819 )
               m_pNodes[5].header.affinity = DVP_CORE_CPU;

            m_pNodes[6].header.kernel = DVP_KN_IMG_THR_LE2MIN_8;
            dvp_knode_to(&m_pNodes[6], DVP_Threshold_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[6], DVP_Threshold_t)->output = m_images[10];
            dvp_knode_to(&m_pNodes[6], DVP_Threshold_t)->thresh = 100;
            //m_pNodes[6].header.affinity = DVP_CORE_CPU;

            m_pNodes[7].header.kernel = DVP_KN_VLIB_CCL;
            dvp_knode_to(&m_pNodes[7], DVP_CCL_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[7], DVP_CCL_t)->output = m_images[11];
            dvp_knode_to(&m_pNodes[7], DVP_CCL_t)->minBlobAreaInPixels = 7;
            dvp_knode_to(&m_pNodes[7], DVP_CCL_t)->connected8Flag = DVP_CCL_8_CONNECTED;
            dvp_knode_to(&m_pNodes[7], DVP_CCL_t)->numCCs= 0; // Initialize to 0
            //m_pNodes[7].header.affinity = DVP_CORE_CPU;

            DVP_Image_Fill(&m_images[3], (DVP_S08*)gaussian_7x7, 7*7*sizeof(DVP_S08)); // Copying the mask

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(11))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_inputCanny");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_lumaCanny");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_cannySmooth");
                ImageDebug_Init(&m_imgdbg[3], &m_images[4], m_imgdbg_path, "03_canny2dgradX");
                ImageDebug_Init(&m_imgdbg[4], &m_images[5], m_imgdbg_path, "04_canny2dgradY");
                ImageDebug_Init(&m_imgdbg[5], &m_images[6], m_imgdbg_path, "05_canny2dmag");
                ImageDebug_Init(&m_imgdbg[6], &m_images[7], m_imgdbg_path, "06_cannyNonma");
                ImageDebug_Init(&m_imgdbg[7], &m_images[8], m_imgdbg_path, "07_dThresh");
                ImageDebug_Init(&m_imgdbg[8], &m_images[9], m_imgdbg_path, "08_intImg");
                ImageDebug_Init(&m_imgdbg[9], &m_images[10], m_imgdbg_path, "09_thresh");
                ImageDebug_Init(&m_imgdbg[10], &m_images[11], m_imgdbg_path, "10_ccl");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);

            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_VrunGraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VRUN)
    DVP_Bounds_t bound = {NULL, 0, 0};
    DVP_S08 maskOnes[3][3] = {{1,1,1},{1,1,1},{1,1,1}};
    DVP_S08 maskBlurNeg3x3[3][3] = {{-5, 20, 17} ,
                                    {19, -28, 13} ,
                                    {-22, 9, 33}};
    DVP_S08 maskBlur7x7[7][7]    = {{5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5}};
    DVP_U08 maskSAD16x16[16][16] ={{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}};

    // 3x3 Gaussian kernel with SQ0.8 coefficients
    const DVP_S08 gaussian_7x7[49] =
    {
        0, 0,  0,  0,  0, 0, 0,
        0, 0,  0,  0,  0, 0, 0,
        0, 0, 16, 32, 16, 0, 0,
        0, 0, 32, 64, 32, 0, 0,
        0, 0, 16, 32, 16, 0, 0,
        0, 0,  0,  0,  0, 0, 0,
        0, 0,  0,  0,  0, 0, 0
    };
    DVP_S08 maskAvg[7] = {1,2,3,4,3,2,1};
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e smallType = DVP_MTYPE_MPUCACHED_VIRTUAL;
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;

#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_MPUNONCACHED_2DTILED;
    smallType = DVP_MTYPE_MPUNONCACHED_1DTILED;
    opType = DVP_MTYPE_DEFAULT;
#endif
    if (m_hDVP)
    {
        if (AllocateImageStructs(66))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_YV24);

            // images 2-7 moved to end of list

            /** Misc**/
            DVP_Image_Init(&m_images[8], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[9], m_width, m_height, FOURCC_RGBP);
            DVP_Image_Init(&m_images[10], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[11], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[12], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[13], m_width, m_height, FOURCC_IYUV);
            /** Morph**/
            DVP_Image_Init(&m_images[14], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[15], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[16], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[17], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[18], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[19], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[20], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[21], 3, 3, FOURCC_Y800);
            /** NonMax**/
            DVP_Image_Init(&m_images[22], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[23], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[24], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[25], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[26], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[27], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[28], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[29], 3, 3, FOURCC_Y800);
            DVP_Image_Init(&m_images[30], 5, 5, FOURCC_Y800);
            DVP_Image_Init(&m_images[31], 7, 7, FOURCC_Y800);
            /** Canny**/
            DVP_Image_Init(&m_images[32], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[33], 7, 7, FOURCC_Y800);
            DVP_Image_Init(&m_images[34], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[35], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[36], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[37], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[38], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[39], m_width, m_height, FOURCC_Y32);

            // image 40 moved to end of list

            /** New **/
            DVP_Image_Init(&m_images[41], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[42], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[43], 7, 1, FOURCC_Y800);
            DVP_Image_Init(&m_images[44], 1, 7, FOURCC_Y800);

            // images 45-46 moved to end of list

            DVP_Image_Init(&m_images[47], 16, 16, FOURCC_Y800);            //sad ref image
            DVP_Image_Init(&m_images[48], m_width, m_height, FOURCC_Y800); //sad 8x8
            DVP_Image_Init(&m_images[49], m_width, m_height, FOURCC_Y800); //sad 16x16
            DVP_Image_Init(&m_images[50], m_width, m_height, FOURCC_Y800); //sad 3x3
            DVP_Image_Init(&m_images[51], m_width, m_height, FOURCC_Y800); //sad 5x5
            DVP_Image_Init(&m_images[52], m_width, m_height, FOURCC_Y800); //sad 7x7
            DVP_Image_Init(&m_images[53], m_width, m_height, FOURCC_RGBP);
            /** Imglib **/
            DVP_Image_Init(&m_images[54], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[55], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[56], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[57], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[58], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[59], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[60], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[61], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[62], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[63], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[64], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[65], m_width, m_height, FOURCC_Y800);

            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[6], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[7], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[40], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[45], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[46], m_width, m_height, FOURCC_Y16);
            /** Misc**/
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0],  camType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1],  opType) ||

                // images 2-7 moved to end of list

                /** Misc **/
#if defined(DVP_USE_ION) || defined(DVP_USE_TILER) || defined(DVP_USE_BO)
                !DVP_Image_Alloc(m_hDVP, &m_images[8],  DVP_MTYPE_MPUNONCACHED_2DTILED) ||
#else
                !DVP_Image_Alloc(m_hDVP, &m_images[8],  opType) ||
#endif
                !DVP_Image_Alloc(m_hDVP, &m_images[9],  opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[10], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[11], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[12], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[13], opType) ||
                /** Morph**/
                !DVP_Image_Alloc(m_hDVP, &m_images[14], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[15], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[16], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[17], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[18], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[19], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[20], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[21], smallType) ||      // Mask can be in virtual space since it is copied
                /** NonMax**/
                !DVP_Image_Alloc(m_hDVP, &m_images[22], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[23], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[24], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[25], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[26], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[27], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[28], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[29], smallType) ||       // Mask can be in virtual space since it is copied
                !DVP_Image_Alloc(m_hDVP, &m_images[30], smallType) ||       // Mask can be in virtual space since it is copied
                !DVP_Image_Alloc(m_hDVP, &m_images[31], smallType) ||       // Mask can be in virtual space since it is copied
                /** Canny**/
                !DVP_Image_Alloc(m_hDVP, &m_images[32], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[33], smallType) ||       // Mask can be in virtual space since it is copied
                !DVP_Image_Alloc(m_hDVP, &m_images[34], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[35], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[36], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[37], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[38], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[39], opType) ||

                // image 40 moved to end of list

                /** New Requests**/
                !DVP_Image_Alloc(m_hDVP, &m_images[41], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[42], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[43], smallType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[44], smallType) ||

                // images 45-46 moved to end of list

                !DVP_Image_Alloc(m_hDVP, &m_images[47], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[48], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[49], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[50], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[51], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[52], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[53], opType) ||
                /** Imglib **/
                !DVP_Image_Alloc(m_hDVP, &m_images[54], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[55], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[56], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[57], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[58], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[59], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[60], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[61], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[62], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[63], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[64], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[65], opType) ||

                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[40], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[45], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[46], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(56) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_VRUN_UYVY_TO_YUV444p;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];

            m_pNodes[1].header.kernel = DVP_KN_NOOP;
            m_pNodes[2].header.kernel = DVP_KN_NOOP;

            /*********

            **********/
            m_pNodes[3].header.kernel = DVP_KN_VRUN_XYXY_TO_Y800;
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->output = m_images[8];

            m_pNodes[4].header.kernel = DVP_KN_VRUN_UYVY_TO_RGBp;
            dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->output = m_images[9];

            m_pNodes[5].header.kernel = DVP_KN_VRUN_IIR_HORZ;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->output = m_images[10];
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->weight = 2000;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->bounds[0] = bound;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->bounds[1] = bound;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->scratch = m_images[11];

            m_pNodes[6].header.kernel = DVP_KN_VRUN_IIR_VERT;
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->output = m_images[12];
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->weight = 2000;
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->bounds[0] = bound;
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->bounds[1] = bound;
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->scratch = m_images[11];

            m_pNodes[7].header.kernel = DVP_KN_VRUN_UYVY_TO_YUV420p;
            dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->output = m_images[13];

            /*********
              Morph
            **********/
            m_pNodes[8].header.kernel = DVP_KN_THRESHOLD;         // Then threshold for binary image functions
            dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->input = m_images[8];       // Using Luma from Luma Extract function
            dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->output = m_images[14];     // Thresholded output

            m_pNodes[9].header.kernel = DVP_KN_VRUN_ERODE_CROSS;
            dvp_knode_to(&m_pNodes[9], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[9], DVP_Morphology_t)->output = m_images[15];

            m_pNodes[10].header.kernel = DVP_KN_VRUN_ERODE_SQUARE;
            dvp_knode_to(&m_pNodes[10], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[10], DVP_Morphology_t)->output = m_images[16];

            m_pNodes[11].header.kernel = DVP_KN_VRUN_ERODE_MASK;
            dvp_knode_to(&m_pNodes[11], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[11], DVP_Morphology_t)->output = m_images[17];
            dvp_knode_to(&m_pNodes[11], DVP_Morphology_t)->mask = m_images[21];

            m_pNodes[12].header.kernel = DVP_KN_VRUN_DILATE_CROSS;
            dvp_knode_to(&m_pNodes[12], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[12], DVP_Morphology_t)->output = m_images[18];

            m_pNodes[13].header.kernel = DVP_KN_VRUN_DILATE_SQUARE;
            dvp_knode_to(&m_pNodes[13], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[13], DVP_Morphology_t)->output = m_images[19];

            m_pNodes[14].header.kernel = DVP_KN_VRUN_DILATE_MASK;
            dvp_knode_to(&m_pNodes[14], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[14], DVP_Morphology_t)->output = m_images[20];
            dvp_knode_to(&m_pNodes[14], DVP_Morphology_t)->mask = m_images[21];

            DVP_Image_Fill(&m_images[21], (DVP_S08 *)maskOnes, 3*3*sizeof(DVP_S08)); //  Setting the mask to be a square (all 1s)

            /*********
              NonMax
            **********/
            m_pNodes[15].header.kernel = DVP_KN_XSTRIDE_CONVERT; // From common kernels
            dvp_knode_to(&m_pNodes[15], DVP_Transform_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[15], DVP_Transform_t)->output = m_images[22];

            m_pNodes[16].header.kernel = DVP_KN_VRUN_NONMAXSUPPRESS_3x3_S16;
            dvp_knode_to(&m_pNodes[16], DVP_Threshold_t)->input = m_images[22];
            dvp_knode_to(&m_pNodes[16], DVP_Threshold_t)->output = m_images[23];
            dvp_knode_to(&m_pNodes[16], DVP_Threshold_t)->thresh = 100;
            if(dvp_knode_to(&m_pNodes[16], DVP_Threshold_t)->input.width > 992)
                m_pNodes[16].header.kernel = DVP_KN_NOOP;

            m_pNodes[17].header.kernel = DVP_KN_VRUN_NONMAXSUPPRESS_5x5_S16;
            dvp_knode_to(&m_pNodes[17], DVP_Threshold_t)->input = m_images[22];
            dvp_knode_to(&m_pNodes[17], DVP_Threshold_t)->output = m_images[24];
            dvp_knode_to(&m_pNodes[17], DVP_Threshold_t)->thresh = 100;
            if(dvp_knode_to(&m_pNodes[17], DVP_Threshold_t)->input.width > 992)
                m_pNodes[17].header.kernel = DVP_KN_NOOP;

            m_pNodes[18].header.kernel = DVP_KN_VRUN_NONMAXSUPPRESS_7x7_S16;
            dvp_knode_to(&m_pNodes[18], DVP_Threshold_t)->input = m_images[22];
            dvp_knode_to(&m_pNodes[18], DVP_Threshold_t)->output = m_images[25];
            dvp_knode_to(&m_pNodes[18], DVP_Threshold_t)->thresh = 100;
            if(dvp_knode_to(&m_pNodes[18], DVP_Threshold_t)->input.width > 768)
                m_pNodes[18].header.kernel = DVP_KN_NOOP;

            m_pNodes[19].header.kernel = DVP_KN_VRUN_CONV_3x3;
            dvp_knode_to(&m_pNodes[19], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[19], DVP_ImageConvolution_t)->output = m_images[26];
            dvp_knode_to(&m_pNodes[19], DVP_ImageConvolution_t)->mask = m_images[29];
            dvp_knode_to(&m_pNodes[19], DVP_ImageConvolution_t)->shiftMask = 6;

            m_pNodes[20].header.kernel = DVP_KN_VRUN_CONV_5x5;
            dvp_knode_to(&m_pNodes[20], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[20], DVP_ImageConvolution_t)->output = m_images[27];
            dvp_knode_to(&m_pNodes[20], DVP_ImageConvolution_t)->mask = m_images[30];
            dvp_knode_to(&m_pNodes[20], DVP_ImageConvolution_t)->shiftMask = 7;

            m_pNodes[21].header.kernel = DVP_KN_VRUN_CONV_7x7;
            dvp_knode_to(&m_pNodes[21], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[21], DVP_ImageConvolution_t)->output = m_images[28];
            dvp_knode_to(&m_pNodes[21], DVP_ImageConvolution_t)->mask = m_images[31];
            dvp_knode_to(&m_pNodes[21], DVP_ImageConvolution_t)->shiftMask = 8;

            DVP_Image_Fill(&m_images[29], (DVP_S08*)maskBlurNeg3x3, 3*3*sizeof(DVP_S08));
            DVP_Image_Fill(&m_images[30], (DVP_S08*)maskBlur7x7,    5*5*sizeof(DVP_S08));
            DVP_Image_Fill(&m_images[31], (DVP_S08*)maskBlur7x7,    7*7*sizeof(DVP_S08));

            /*********
              Canny
            **********/
            m_pNodes[22].header.kernel = DVP_KN_VRUN_CANNY_IMAGE_SMOOTHING;
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_t)->output = m_images[32];
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_t)->mask = m_images[33];
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_t)->shiftMask = 8;

            m_pNodes[23].header.kernel = DVP_KN_VRUN_CANNY_2D_GRADIENT;
            dvp_knode_to(&m_pNodes[23], DVP_Canny2dGradient_t)->input = m_images[32];
            dvp_knode_to(&m_pNodes[23], DVP_Canny2dGradient_t)->outGradX = m_images[34];
            dvp_knode_to(&m_pNodes[23], DVP_Canny2dGradient_t)->outGradY = m_images[35];
            dvp_knode_to(&m_pNodes[23], DVP_Canny2dGradient_t)->outMag = m_images[36];

            m_pNodes[24].header.kernel = DVP_KN_VRUN_CANNY_NONMAX_SUPPRESSION;
            dvp_knode_to(&m_pNodes[24], DVP_CannyNonMaxSuppression_t)->inGradX = m_images[34];
            dvp_knode_to(&m_pNodes[24], DVP_CannyNonMaxSuppression_t)->inGradY = m_images[35];
            dvp_knode_to(&m_pNodes[24], DVP_CannyNonMaxSuppression_t)->inMag = m_images[36];
            dvp_knode_to(&m_pNodes[24], DVP_CannyNonMaxSuppression_t)->output = m_images[37];

            m_pNodes[25].header.kernel = DVP_KN_VRUN_CANNY_HYST_THRESHHOLD;
            dvp_knode_to(&m_pNodes[25], DVP_CannyHystThresholding_t)->inMag = m_images[36];
            dvp_knode_to(&m_pNodes[25], DVP_CannyHystThresholding_t)->inEdgeMap = m_images[37];
            dvp_knode_to(&m_pNodes[25], DVP_CannyHystThresholding_t)->output = m_images[38];
            dvp_knode_to(&m_pNodes[25], DVP_CannyHystThresholding_t)->loThresh = 35;
            dvp_knode_to(&m_pNodes[25], DVP_CannyHystThresholding_t)->hiThresh = 122;

            m_pNodes[26].header.kernel = DVP_KN_VRUN_INTEGRAL_IMAGE_8;
            dvp_knode_to(&m_pNodes[26], DVP_Transform_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[26], DVP_Transform_t)->output = m_images[39];
            if(dvp_knode_to(&m_pNodes[26], DVP_Transform_t)->input.width> 2049 || dvp_knode_to(&m_pNodes[26], DVP_Transform_t)->input.height> 819 )
                m_pNodes[26].header.kernel = DVP_KN_NOOP;

            DVP_Image_Fill(&m_images[33], (DVP_S08*)gaussian_7x7, 7*7*sizeof(DVP_S08)); // Copying the mask

            m_pNodes[27].header.kernel = DVP_KN_VRUN_CONV_MxN;
            dvp_knode_to(&m_pNodes[27], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[27], DVP_ImageConvolution_t)->output = m_images[41];
            dvp_knode_to(&m_pNodes[27], DVP_ImageConvolution_t)->mask = m_images[43];
            dvp_knode_to(&m_pNodes[27], DVP_ImageConvolution_t)->shiftMask = 4;

            m_pNodes[28].header.kernel = DVP_KN_VRUN_CONV_MxN;
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->output = m_images[42];
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->mask = m_images[44];
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->shiftMask = 4;

            DVP_Image_Fill(&m_images[43], (DVP_S08*)maskAvg, 7*sizeof(DVP_S08)); // Copying the mask
            DVP_Image_Fill(&m_images[44], (DVP_S08*)maskAvg, 7*sizeof(DVP_S08)); // Copying the mask

            m_pNodes[29].header.kernel = DVP_KN_VRUN_SAD_8x8;
            dvp_knode_to(&m_pNodes[29], DVP_SAD_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[29], DVP_SAD_t)->output = m_images[48];
            dvp_knode_to(&m_pNodes[29], DVP_SAD_t)->refImg = m_images[47];
            dvp_knode_to(&m_pNodes[29], DVP_SAD_t)->shiftMask = 4;
            dvp_knode_to(&m_pNodes[29], DVP_SAD_t)->refPitch = 8;
            dvp_knode_to(&m_pNodes[29], DVP_SAD_t)->refStartOffset = 0;

            m_pNodes[30].header.kernel = DVP_KN_VRUN_SAD_16x16;
            dvp_knode_to(&m_pNodes[30], DVP_SAD_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[30], DVP_SAD_t)->output = m_images[49];
            dvp_knode_to(&m_pNodes[30], DVP_SAD_t)->refImg = m_images[47];
            dvp_knode_to(&m_pNodes[30], DVP_SAD_t)->shiftMask = 8;
            dvp_knode_to(&m_pNodes[30], DVP_SAD_t)->refPitch = 16;
            dvp_knode_to(&m_pNodes[30], DVP_SAD_t)->refStartOffset = 0;

            m_pNodes[31].header.kernel = DVP_KN_VRUN_SAD_3x3;
            dvp_knode_to(&m_pNodes[31], DVP_SAD_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[31], DVP_SAD_t)->output = m_images[50];
            dvp_knode_to(&m_pNodes[31], DVP_SAD_t)->refImg = m_images[47];
            dvp_knode_to(&m_pNodes[31], DVP_SAD_t)->refPitch = 3;
            dvp_knode_to(&m_pNodes[31], DVP_SAD_t)->refStartOffset = 0;

            m_pNodes[32].header.kernel = DVP_KN_VRUN_SAD_5x5;
            dvp_knode_to(&m_pNodes[32], DVP_SAD_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[32], DVP_SAD_t)->output = m_images[51];
            dvp_knode_to(&m_pNodes[32], DVP_SAD_t)->refImg = m_images[47];
            dvp_knode_to(&m_pNodes[32], DVP_SAD_t)->refPitch = 5;
            dvp_knode_to(&m_pNodes[32], DVP_SAD_t)->refStartOffset = 0;

            m_pNodes[33].header.kernel = DVP_KN_VRUN_SAD_7x7;
            dvp_knode_to(&m_pNodes[33], DVP_SAD_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[33], DVP_SAD_t)->output = m_images[52];
            dvp_knode_to(&m_pNodes[33], DVP_SAD_t)->refImg = m_images[47];
            dvp_knode_to(&m_pNodes[33], DVP_SAD_t)->refPitch = 7;
            dvp_knode_to(&m_pNodes[33], DVP_SAD_t)->refStartOffset = 0;

            DVP_Image_Fill(&m_images[47], (DVP_S08*)maskSAD16x16,   16*16*sizeof(DVP_S08));

            m_pNodes[34].header.kernel = DVP_KN_VRUN_YUV444p_TO_RGBp;
            dvp_knode_to(&m_pNodes[34], DVP_Transform_t)->input  = m_images[1];
            dvp_knode_to(&m_pNodes[34], DVP_Transform_t)->output = m_images[53];

            /*********
              Imglib
            **********/
            m_pNodes[35].header.kernel = DVP_KN_XSTRIDE_SHIFT;
            dvp_knode_to(&m_pNodes[35], DVP_Transform_t)->input = m_images[8];        // Use 8bit luma
            dvp_knode_to(&m_pNodes[35], DVP_Transform_t)->output = m_images[54];       // Output 16bit luma

            m_pNodes[36].header.kernel = DVP_KN_VRUN_THR_GT2MAX_8;
            dvp_knode_to(&m_pNodes[36], DVP_Threshold_t)->input = m_images[8];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[36], DVP_Threshold_t)->output = m_images[55];
            dvp_knode_to(&m_pNodes[36], DVP_Threshold_t)->thresh = 128;

            m_pNodes[37].header.kernel = DVP_KN_VRUN_THR_GT2THR_8;
            dvp_knode_to(&m_pNodes[37], DVP_Threshold_t)->input = m_images[8];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[37], DVP_Threshold_t)->output = m_images[56];
            dvp_knode_to(&m_pNodes[37], DVP_Threshold_t)->thresh = 128;

            m_pNodes[38].header.kernel = DVP_KN_VRUN_THR_LE2MIN_8;
            dvp_knode_to(&m_pNodes[38], DVP_Threshold_t)->input = m_images[8];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[38], DVP_Threshold_t)->output = m_images[57];
            dvp_knode_to(&m_pNodes[38], DVP_Threshold_t)->thresh = 128;

            m_pNodes[39].header.kernel = DVP_KN_VRUN_THR_LE2THR_8;
            dvp_knode_to(&m_pNodes[39], DVP_Threshold_t)->input = m_images[8];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[39], DVP_Threshold_t)->output = m_images[58];
            dvp_knode_to(&m_pNodes[39], DVP_Threshold_t)->thresh = 128;

            m_pNodes[40].header.kernel = DVP_KN_VRUN_THR_GT2MAX_16;
            dvp_knode_to(&m_pNodes[40], DVP_Threshold_t)->input = m_images[54];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[40], DVP_Threshold_t)->output = m_images[59];
            dvp_knode_to(&m_pNodes[40], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;

            m_pNodes[41].header.kernel = DVP_KN_VRUN_THR_GT2THR_16;
            dvp_knode_to(&m_pNodes[41], DVP_Threshold_t)->input = m_images[54];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[41], DVP_Threshold_t)->output = m_images[60];
            dvp_knode_to(&m_pNodes[41], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;

            m_pNodes[42].header.kernel = DVP_KN_VRUN_THR_LE2MIN_16;
            dvp_knode_to(&m_pNodes[42], DVP_Threshold_t)->input = m_images[54];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[42], DVP_Threshold_t)->output = m_images[61];
            dvp_knode_to(&m_pNodes[42], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;

            m_pNodes[43].header.kernel = DVP_KN_VRUN_THR_LE2THR_16;
            dvp_knode_to(&m_pNodes[43], DVP_Threshold_t)->input = m_images[54];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[43], DVP_Threshold_t)->output = m_images[62];
            dvp_knode_to(&m_pNodes[43], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;

            m_pNodes[44].header.kernel = DVP_KN_VRUN_SOBEL_3x3_8s;
            dvp_knode_to(&m_pNodes[44], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[44], DVP_Transform_t)->output = m_images[63];

            m_pNodes[45].header.kernel = DVP_KN_VRUN_SOBEL_3x3_8;
            dvp_knode_to(&m_pNodes[45], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[45], DVP_Transform_t)->output = m_images[64];

            m_pNodes[46].header.kernel = DVP_KN_VRUN_SOBEL_5x5_8s;
            dvp_knode_to(&m_pNodes[46], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[46], DVP_Transform_t)->output = m_images[65];

            m_pNodes[47].header.kernel = DVP_KN_VRUN_SOBEL_5x5_8;
            dvp_knode_to(&m_pNodes[47], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[47], DVP_Transform_t)->output = m_images[2];

            m_pNodes[48].header.kernel = DVP_KN_VRUN_SOBEL_7x7_8s;
            dvp_knode_to(&m_pNodes[48], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[48], DVP_Transform_t)->output = m_images[3];

            m_pNodes[49].header.kernel = DVP_KN_VRUN_SOBEL_7x7_8;
            dvp_knode_to(&m_pNodes[49], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[49], DVP_Transform_t)->output = m_images[4];

            m_pNodes[50].header.kernel = DVP_KN_VRUN_SOBEL_3x3_16s;
            dvp_knode_to(&m_pNodes[50], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[50], DVP_Transform_t)->output = m_images[5];

            m_pNodes[51].header.kernel = DVP_KN_VRUN_SOBEL_3x3_16;
            dvp_knode_to(&m_pNodes[51], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[51], DVP_Transform_t)->output = m_images[6];

            m_pNodes[52].header.kernel = DVP_KN_VRUN_SOBEL_5x5_16s;
            dvp_knode_to(&m_pNodes[52], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[52], DVP_Transform_t)->output = m_images[7];

            m_pNodes[53].header.kernel = DVP_KN_VRUN_SOBEL_5x5_16;
            dvp_knode_to(&m_pNodes[53], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[53], DVP_Transform_t)->output = m_images[40];

            m_pNodes[54].header.kernel = DVP_KN_VRUN_SOBEL_7x7_16s;
            dvp_knode_to(&m_pNodes[54], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[54], DVP_Transform_t)->output = m_images[45];

            m_pNodes[55].header.kernel = DVP_KN_VRUN_SOBEL_7x7_16;
            dvp_knode_to(&m_pNodes[55], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[55], DVP_Transform_t)->output = m_images[46];

#if 0
            TBD:
                DVP_KN_VRUN_NV12_TO_YUV444p
                DVP_KN_VRUN_HARRIS_CORNERS
                DVP_KN_VRUN_HARRIS_SCORE_7x7
                DVP_KN_VRUN_BLOCK_MAXIMA
                DVP_KN_VRUN_NMS_STEP1
#endif

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }

        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(58))
            {
                /** Misc**/
                ImageDebug_Init(&m_imgdbg[0],  &m_images[0],  m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1],  &m_images[1],  m_imgdbg_path, "01_YUV444");

                // images 2-7 moved to end of list

                /** **/
                ImageDebug_Init(&m_imgdbg[8],  &m_images[8],  m_imgdbg_path, "08_luma");
                ImageDebug_Init(&m_imgdbg[9],  &m_images[9],  m_imgdbg_path, "09_RGBpl");
                ImageDebug_Init(&m_imgdbg[10], &m_images[10], m_imgdbg_path, "10_IIRH");
                ImageDebug_Init(&m_imgdbg[11], &m_images[12], m_imgdbg_path, "11_IIRV");
                ImageDebug_Init(&m_imgdbg[12], &m_images[13], m_imgdbg_path, "12_YUV420");
                /** Morph**/
                ImageDebug_Init(&m_imgdbg[13], &m_images[14], m_imgdbg_path, "13_bitimgThresh");
                ImageDebug_Init(&m_imgdbg[14], &m_images[15], m_imgdbg_path, "14_bitimgErodeX");
                ImageDebug_Init(&m_imgdbg[15], &m_images[16], m_imgdbg_path, "15_bitimgErodeSq");
                ImageDebug_Init(&m_imgdbg[16], &m_images[17], m_imgdbg_path, "16_bitimgErodeMsk");
                ImageDebug_Init(&m_imgdbg[17], &m_images[18], m_imgdbg_path, "17_bitimgDilateX");
                ImageDebug_Init(&m_imgdbg[18], &m_images[19], m_imgdbg_path, "18_bitimgDilateSq");
                ImageDebug_Init(&m_imgdbg[19], &m_images[20], m_imgdbg_path, "19_bitimgDilateMsk");
                /** NonMax**/
                ImageDebug_Init(&m_imgdbg[20], &m_images[22], m_imgdbg_path, "20_16bitStride");
                ImageDebug_Init(&m_imgdbg[21], &m_images[23], m_imgdbg_path, "21_nonma3");
                ImageDebug_Init(&m_imgdbg[22], &m_images[24], m_imgdbg_path, "22_nonma5");
                ImageDebug_Init(&m_imgdbg[23], &m_images[25], m_imgdbg_path, "23_nonma7");
                ImageDebug_Init(&m_imgdbg[24], &m_images[26], m_imgdbg_path, "24_imgconv3");
                ImageDebug_Init(&m_imgdbg[25], &m_images[27], m_imgdbg_path, "25_imgconv5");
                ImageDebug_Init(&m_imgdbg[26], &m_images[28], m_imgdbg_path, "26_imgconv7");
                /** Canny**/
                ImageDebug_Init(&m_imgdbg[27], &m_images[32], m_imgdbg_path, "27_cannySmooth");
                ImageDebug_Init(&m_imgdbg[28], &m_images[34], m_imgdbg_path, "28_canny2dgradX");
                ImageDebug_Init(&m_imgdbg[29], &m_images[35], m_imgdbg_path, "29_canny2dgradY");
                ImageDebug_Init(&m_imgdbg[30], &m_images[36], m_imgdbg_path, "30_canny2dmag");
                ImageDebug_Init(&m_imgdbg[31], &m_images[37], m_imgdbg_path, "31_cannyNonma");
                ImageDebug_Init(&m_imgdbg[32], &m_images[38], m_imgdbg_path, "32_dThresh");
                ImageDebug_Init(&m_imgdbg[33], &m_images[39], m_imgdbg_path, "33_intImg");
                /** New Requests**/

                // image 40 moved to end of list

                ImageDebug_Init(&m_imgdbg[35], &m_images[41], m_imgdbg_path, "35_fir7X1");
                ImageDebug_Init(&m_imgdbg[36], &m_images[42], m_imgdbg_path, "36_fir1X7");

                // images 45-46 moved to end of list

                ImageDebug_Init(&m_imgdbg[39], &m_images[47], m_imgdbg_path, "39_refImg");
                ImageDebug_Init(&m_imgdbg[40], &m_images[48], m_imgdbg_path, "40_SAD_8X8");
                ImageDebug_Init(&m_imgdbg[41], &m_images[49], m_imgdbg_path, "41_SAD_16X16");
                ImageDebug_Init(&m_imgdbg[42], &m_images[50], m_imgdbg_path, "42_SAD_3X3");
                ImageDebug_Init(&m_imgdbg[43], &m_images[51], m_imgdbg_path, "43_SAD_5X5");
                ImageDebug_Init(&m_imgdbg[44], &m_images[52], m_imgdbg_path, "44_SAD_7X7");
                ImageDebug_Init(&m_imgdbg[45], &m_images[53], m_imgdbg_path, "45_RGBpl");
                /** Imglib **/
                ImageDebug_Init(&m_imgdbg[46], &m_images[54], m_imgdbg_path, "46_luma16");
                ImageDebug_Init(&m_imgdbg[47], &m_images[55], m_imgdbg_path, "47_thr8_gtMAX");
                ImageDebug_Init(&m_imgdbg[48], &m_images[56], m_imgdbg_path, "48_thr8_gtTHR");
                ImageDebug_Init(&m_imgdbg[49], &m_images[57], m_imgdbg_path, "49_thr8_leMIN");
                ImageDebug_Init(&m_imgdbg[50], &m_images[58], m_imgdbg_path, "50_thr8_leTHR");
                ImageDebug_Init(&m_imgdbg[51], &m_images[59], m_imgdbg_path, "51_thr16_gtMAX");
                ImageDebug_Init(&m_imgdbg[52], &m_images[60], m_imgdbg_path, "52_thr16_gtTHR");
                ImageDebug_Init(&m_imgdbg[53], &m_images[61], m_imgdbg_path, "53_thr16_leMIN");
                ImageDebug_Init(&m_imgdbg[54], &m_images[62], m_imgdbg_path, "54_thr16_leTHR");
                ImageDebug_Init(&m_imgdbg[55], &m_images[63], m_imgdbg_path, "55_sobel8_3X3_s");
                ImageDebug_Init(&m_imgdbg[56], &m_images[64], m_imgdbg_path, "56_sobel8_3X3");
                ImageDebug_Init(&m_imgdbg[57], &m_images[65], m_imgdbg_path, "57_sobel8_5X5_s");

                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "58_sobel8_5X5");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "59_sobel8_7X7_s");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "60_sobel8_7X7");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "61_sobel16_3X3_s");
                ImageDebug_Init(&m_imgdbg[6], &m_images[6], m_imgdbg_path, "62_sobel16_3X3");
                ImageDebug_Init(&m_imgdbg[7], &m_images[7], m_imgdbg_path, "63_sobel16_5X5_s");
                ImageDebug_Init(&m_imgdbg[34], &m_images[40], m_imgdbg_path, "64_sobel16_5X5");
                ImageDebug_Init(&m_imgdbg[37], &m_images[46], m_imgdbg_path, "65_sobel16_7X7_s");
                ImageDebug_Init(&m_imgdbg[38], &m_images[45], m_imgdbg_path, "66_sobel16_7X7");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_VrunGraphSetup2() // The first VRUN test is too long
{
    status_e status = STATUS_SUCCESS;

#if defined(DVP_USE_VRUN)
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;

    if (m_hDVP)
    {
        if (AllocateImageStructs(14))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800); // Luma
            DVP_Image_Init(&m_images[2], m_width/2, m_height/2, FOURCC_Y800); // Gaussian 3x3
            DVP_Image_Init(&m_images[3], m_width/2, m_height/2, FOURCC_Y800); // Gaussian 5x5
            DVP_Image_Init(&m_images[4], m_width/2, m_height/2, FOURCC_Y800); // Gaussian 7x7
            DVP_Image_Init(&m_images[5], m_width/2, m_height/2, FOURCC_Y800); // GradientH 3x3
            DVP_Image_Init(&m_images[6], m_width/2, m_height/2, FOURCC_Y800); // GradientH 5x5
            DVP_Image_Init(&m_images[7], m_width/2, m_height/2, FOURCC_Y800); // GradientH 7x7
            DVP_Image_Init(&m_images[8], m_width/2, m_height/2, FOURCC_Y800); // GradientV 3x3
            DVP_Image_Init(&m_images[9], m_width/2, m_height/2, FOURCC_Y800); // GradientV 5x5
            DVP_Image_Init(&m_images[10], m_width/2, m_height/2, FOURCC_Y800); // GradientV 7x7
            DVP_Image_Init(&m_images[11], m_width/2, m_height/2, FOURCC_Y800); // ImgPyr lv 1
            DVP_Image_Init(&m_images[12], m_width/4, m_height/4, FOURCC_Y800); // ImgPyr lv 2
            DVP_Image_Init(&m_images[13], m_width/8, m_height/8, FOURCC_Y800); // ImgPyr lv 3

            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[8], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[9], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[10], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateBufferStructs(1))
        {
            DVP_Buffer_Init(&m_buffers[0], 1, m_width*m_height*21/64);  // Img Pyramid
            if (!DVP_Buffer_Alloc(m_hDVP, &m_buffers[0], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(11) && AllocateSections(&m_graphs[0], 1))
        {
            /* Luma Extraction */
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];

            m_pNodes[1].header.kernel = DVP_KN_VRUN_GAUSSIAN_3x3_PYRAMID_8;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[2];

            m_pNodes[2].header.kernel = DVP_KN_VRUN_GAUSSIAN_5x5_PYRAMID_8;
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->output = m_images[3];

            m_pNodes[3].header.kernel = DVP_KN_VRUN_GAUSSIAN_7x7_PYRAMID_8;
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->output = m_images[4];

            m_pNodes[4].header.kernel = DVP_KN_VRUN_GRADIENT_H3x3_PYRAMID_8;
            dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->output = m_images[5];

            m_pNodes[5].header.kernel = DVP_KN_VRUN_GRADIENT_H5x5_PYRAMID_8;
            dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->output = m_images[6];

            m_pNodes[6].header.kernel = DVP_KN_VRUN_GRADIENT_H7x7_PYRAMID_8;
            dvp_knode_to(&m_pNodes[6], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[6], DVP_Transform_t)->output = m_images[7];

            m_pNodes[7].header.kernel = DVP_KN_VRUN_GRADIENT_V3x3_PYRAMID_8;
            dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->output = m_images[8];

            m_pNodes[8].header.kernel = DVP_KN_VRUN_GRADIENT_V5x5_PYRAMID_8;
            dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->output = m_images[9];

            m_pNodes[9].header.kernel = DVP_KN_VRUN_GRADIENT_V7x7_PYRAMID_8;
            dvp_knode_to(&m_pNodes[9], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[9], DVP_Transform_t)->output = m_images[10];

            m_pNodes[10].header.kernel = DVP_KN_VRUN_IMAGE_PYRAMID_8;
            if(m_width % 128)
                m_pNodes[10].header.kernel = DVP_KN_NOOP;

            dvp_knode_to(&m_pNodes[10], DVP_Pyramid_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[10], DVP_Pyramid_t)->output = m_buffers[0];

            m_images[11].pBuffer[0] = m_buffers[0].pData;
            m_images[11].pData[0]   = m_buffers[0].pData;
            m_images[11].y_stride   = m_width/2;
            m_images[11].memType    = m_buffers[0].memType;

            m_images[12].pBuffer[0] = m_buffers[0].pData;
            m_images[12].pData[0]   = m_images[11].pData[0] + m_width*m_height/4;
            m_images[12].y_stride   = m_width/4;
            m_images[12].memType    = m_buffers[0].memType;

            m_images[13].pBuffer[0] = m_buffers[0].pData;
            m_images[13].pData[0]   = m_images[12].pData[0] + m_width*m_height/16;
            m_images[13].y_stride   = m_width/8;
            m_images[13].memType    = m_buffers[0].memType;

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(14))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_gauss3X3");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "03_gauss5X5");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "04_gauss7X7");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "05_gradH3X3");
                ImageDebug_Init(&m_imgdbg[6], &m_images[6], m_imgdbg_path, "06_gradH5X5");
                ImageDebug_Init(&m_imgdbg[7], &m_images[7], m_imgdbg_path, "07_gradH7X7");
                ImageDebug_Init(&m_imgdbg[8], &m_images[8], m_imgdbg_path, "08_gradV3X3");
                ImageDebug_Init(&m_imgdbg[9], &m_images[9], m_imgdbg_path, "09_gradV5X5");
                ImageDebug_Init(&m_imgdbg[10], &m_images[10], m_imgdbg_path, "10_gradV7X7");
                ImageDebug_Init(&m_imgdbg[11], &m_images[11], m_imgdbg_path, "11_pryamidL1");
                ImageDebug_Init(&m_imgdbg[12], &m_images[12], m_imgdbg_path, "12_pryamidL2");
                ImageDebug_Init(&m_imgdbg[13], &m_images[13], m_imgdbg_path, "13_pryamidL3");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}


status_e TestVisionEngine::Test_CommonGraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VRUN) || (defined(DVP_USE_VLIB) && defined(DVP_USE_IMGLIB))
    DVP_Bounds_t bound = {NULL, 0, 0};
    DVP_S08 maskOnes[3][3] = {{1,1,1},{1,1,1},{1,1,1}};
    DVP_S08 maskBlurNeg3x3[3][3] = {{-5, 20, 17} ,
                                    {19, -28, 13} ,
                                    {-22, 9, 33}};
    DVP_S08 maskBlur7x7[7][7]    = {{5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5}};
    // 3x3 Gaussian kernel with SQ0.8 coefficients
    const DVP_S08 gaussian_7x7[49] =
    {
        0, 0,  0,  0,  0, 0, 0,
        0, 0,  0,  0,  0, 0, 0,
        0, 0, 16, 32, 16, 0, 0,
        0, 0, 32, 64, 32, 0, 0,
        0, 0, 16, 32, 16, 0, 0,
        0, 0,  0,  0,  0, 0, 0,
        0, 0,  0,  0,  0, 0, 0
    };
    DVP_S08 maskAvg[7] = {1,2,3,4,3,2,1};
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e smallType = DVP_MTYPE_MPUCACHED_VIRTUAL;

#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_MPUNONCACHED_2DTILED;
    smallType = DVP_MTYPE_MPUNONCACHED_1DTILED;
#endif
    if (m_hDVP)
    {
        if (AllocateImageStructs(69))
        {
            /** **/
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_YU24);

            // images 2-7 moved to end of list

            /** Misc**/
            DVP_Image_Init(&m_images[8], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[9], m_width, m_height, FOURCC_RGBP);
            DVP_Image_Init(&m_images[10], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[11], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[12], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[13], m_width, m_height, FOURCC_IYUV);
            /** Morph**/
            DVP_Image_Init(&m_images[14], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[15], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[16], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[17], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[18], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[19], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[20], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[21], 3, 3, FOURCC_Y800);
            /** NonMax**/
            DVP_Image_Init(&m_images[22], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[23], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[24], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[25], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[26], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[27], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[28], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[29], 3, 3, FOURCC_Y800);
            DVP_Image_Init(&m_images[30], 5, 5, FOURCC_Y800);
            DVP_Image_Init(&m_images[31], 7, 7, FOURCC_Y800);
            /** Canny**/
            DVP_Image_Init(&m_images[32], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[33], 7, 7, FOURCC_Y800);
            DVP_Image_Init(&m_images[34], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[35], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[36], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[37], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[38], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[39], m_width, m_height, FOURCC_Y32);
            DVP_Image_Init(&m_images[40], m_width, m_height, FOURCC_Y800);
            /** New **/
            DVP_Image_Init(&m_images[41], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[42], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[43], 7, 1, FOURCC_Y800);
            DVP_Image_Init(&m_images[44], 1, 7, FOURCC_Y800);
            DVP_Image_Init(&m_images[45], m_width, m_height, FOURCC_Y800); // copy
            DVP_Image_Init(&m_images[46], m_width*2, m_height*2, FOURCC_Y800); // pad
            DVP_Image_Init(&m_images[47], m_width/2-15, m_height/2-15, FOURCC_Y800); // crop
            DVP_Image_Init(&m_images[48], m_width, m_height, FOURCC_Y800); // crop + pad
            DVP_Image_Init(&m_images[49], m_width, m_height, FOURCC_Y800); // resv
            DVP_Image_Init(&m_images[50], m_width, m_height, FOURCC_Y800); // resv
            DVP_Image_Init(&m_images[51], m_width, m_height, FOURCC_Y800); // resv
            DVP_Image_Init(&m_images[52], m_width, m_height, FOURCC_Y800); // resv
            DVP_Image_Init(&m_images[53], m_width, m_height, FOURCC_RGBP);
            /** Imglib **/
            DVP_Image_Init(&m_images[54], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[55], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[56], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[57], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[58], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[59], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[60], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[61], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[62], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[63], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[64], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[65], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[66], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[67], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[68], m_width, m_height, FOURCC_Y800);

            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[6], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[7], m_width, m_height, FOURCC_Y16);

            /** Misc**/
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0],   camType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1],  DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2],  DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3],  DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4],  DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5],  DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6],  DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7],  DVP_MTYPE_DEFAULT) ||
                /** Misc **/
#if defined(DVP_USE_ION) || defined(DVP_USE_TILER) || defined(DVP_USE_BO)
                !DVP_Image_Alloc(m_hDVP, &m_images[8],  DVP_MTYPE_MPUNONCACHED_2DTILED) ||
#else
                !DVP_Image_Alloc(m_hDVP, &m_images[8],  DVP_MTYPE_DEFAULT) ||
#endif
                !DVP_Image_Alloc(m_hDVP, &m_images[9],  DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[10], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[11], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[12], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[13], DVP_MTYPE_DEFAULT) ||
                /** Morph**/
                !DVP_Image_Alloc(m_hDVP, &m_images[14], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[15], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[16], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[17], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[18], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[19], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[20], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[21], smallType) ||      // Mask can be in virtual space since it is copied
                /** NonMax**/
                !DVP_Image_Alloc(m_hDVP, &m_images[22], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[23], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[24], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[25], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[26], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[27], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[28], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[29], smallType) ||       // Mask can be in virtual space since it is copied
                !DVP_Image_Alloc(m_hDVP, &m_images[30], smallType) ||       // Mask can be in virtual space since it is copied
                !DVP_Image_Alloc(m_hDVP, &m_images[31], smallType) ||       // Mask can be in virtual space since it is copied
                /** Canny**/
                !DVP_Image_Alloc(m_hDVP, &m_images[32], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[33], smallType) ||       // Mask can be in virtual space since it is copied
                !DVP_Image_Alloc(m_hDVP, &m_images[34], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[35], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[36], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[37], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[38], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[39], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[40], DVP_MTYPE_DEFAULT) ||
                /** New Requests**/
                !DVP_Image_Alloc(m_hDVP, &m_images[41], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[42], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[43], smallType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[44], smallType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[45], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[46], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[47], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[48], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[49], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[50], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[51], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[52], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[53], DVP_MTYPE_DEFAULT) ||
                /** Imglib **/
                !DVP_Image_Alloc(m_hDVP, &m_images[54], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[55], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[56], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[57], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[58], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[59], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[60], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[61], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[62], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[63], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[64], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[65], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[66], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[67], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[68], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;

        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(56) && AllocateSections(&m_graphs[0], 1))
        {
            /*********
              Misc
            **********/
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_UYVY_TO_YUV444p;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];

            m_pNodes[1].header.kernel = DVP_KN_NOOP;
            m_pNodes[2].header.kernel = DVP_KN_NOOP;

            /*********

            **********/
            m_pNodes[3].header.kernel = DVP_KN_XYXY_TO_Y800;
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->output = m_images[8];

            m_pNodes[4].header.kernel = DVP_KN_UYVY_TO_RGBp;
            dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->output = m_images[9];

            m_pNodes[5].header.kernel = DVP_KN_IIR_HORZ;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->output = m_images[10];
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->weight = 2000;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->bounds[0] = bound;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->bounds[1] = bound;
            dvp_knode_to(&m_pNodes[5], DVP_IIR_t)->scratch = m_images[11];

            m_pNodes[6].header.kernel = DVP_KN_IIR_VERT;
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->output = m_images[12];
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->weight = 2000;
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->bounds[0] = bound;
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->bounds[1] = bound;
            dvp_knode_to(&m_pNodes[6], DVP_IIR_t)->scratch = m_images[11];

            m_pNodes[7].header.kernel = DVP_KN_UYVY_TO_YUV420p;
            dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->output = m_images[13];

            /*********
              Morph
            **********/
            m_pNodes[8].header.kernel = DVP_KN_THRESHOLD;         // Then threshold for binary image functions
            dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->input = m_images[8];       // Using Luma from Luma Extract function
            dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->output = m_images[14];     // Thresholded output

            m_pNodes[9].header.kernel = DVP_KN_ERODE_CROSS;
            dvp_knode_to(&m_pNodes[9], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[9], DVP_Morphology_t)->output = m_images[15];

            m_pNodes[10].header.kernel = DVP_KN_ERODE_SQUARE;
            dvp_knode_to(&m_pNodes[10], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[10], DVP_Morphology_t)->output = m_images[16];

            m_pNodes[11].header.kernel = DVP_KN_ERODE_MASK;
            dvp_knode_to(&m_pNodes[11], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[11], DVP_Morphology_t)->output = m_images[17];
            dvp_knode_to(&m_pNodes[11], DVP_Morphology_t)->mask = m_images[21];

            m_pNodes[12].header.kernel = DVP_KN_DILATE_CROSS;
            dvp_knode_to(&m_pNodes[12], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[12], DVP_Morphology_t)->output = m_images[18];

            m_pNodes[13].header.kernel = DVP_KN_DILATE_SQUARE;
            dvp_knode_to(&m_pNodes[13], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[13], DVP_Morphology_t)->output = m_images[19];

            m_pNodes[14].header.kernel = DVP_KN_DILATE_MASK;
            dvp_knode_to(&m_pNodes[14], DVP_Morphology_t)->input = m_images[14];
            dvp_knode_to(&m_pNodes[14], DVP_Morphology_t)->output = m_images[20];
            dvp_knode_to(&m_pNodes[14], DVP_Morphology_t)->mask = m_images[21];

            DVP_Image_Fill(&m_images[21], (DVP_S08 *)maskOnes, 3*3*sizeof(DVP_S08)); //  Setting the mask to be a square (all 1s)

            /*********
              NonMax
            **********/
            m_pNodes[15].header.kernel = DVP_KN_XSTRIDE_CONVERT;
            dvp_knode_to(&m_pNodes[15], DVP_Transform_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[15], DVP_Transform_t)->output = m_images[22];

            m_pNodes[16].header.kernel = DVP_KN_NONMAXSUPPRESS_3x3_S16;
            dvp_knode_to(&m_pNodes[16], DVP_Threshold_t)->input = m_images[22];
            dvp_knode_to(&m_pNodes[16], DVP_Threshold_t)->output = m_images[23];
            dvp_knode_to(&m_pNodes[16], DVP_Threshold_t)->thresh = 100;
            if(dvp_knode_to(&m_pNodes[16], DVP_Threshold_t)->input.width > 992)
               m_pNodes[16].header.affinity = DVP_CORE_CPU;

            m_pNodes[17].header.kernel = DVP_KN_NONMAXSUPPRESS_5x5_S16;
            dvp_knode_to(&m_pNodes[17], DVP_Threshold_t)->input = m_images[22];
            dvp_knode_to(&m_pNodes[17], DVP_Threshold_t)->output = m_images[24];
            dvp_knode_to(&m_pNodes[17], DVP_Threshold_t)->thresh = 100;
            if(dvp_knode_to(&m_pNodes[17], DVP_Threshold_t)->input.width > 992)
               m_pNodes[17].header.affinity = DVP_CORE_CPU;

            m_pNodes[18].header.kernel = DVP_KN_NONMAXSUPPRESS_7x7_S16;
            dvp_knode_to(&m_pNodes[18], DVP_Threshold_t)->input = m_images[22];
            dvp_knode_to(&m_pNodes[18], DVP_Threshold_t)->output = m_images[25];
            dvp_knode_to(&m_pNodes[18], DVP_Threshold_t)->thresh = 100;
            if(dvp_knode_to(&m_pNodes[18], DVP_Threshold_t)->input.width > 768)
               m_pNodes[18].header.affinity = DVP_CORE_CPU;

            m_pNodes[19].header.kernel = DVP_KN_CONV_3x3;
            dvp_knode_to(&m_pNodes[19], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[19], DVP_ImageConvolution_t)->output = m_images[26];
            dvp_knode_to(&m_pNodes[19], DVP_ImageConvolution_t)->mask = m_images[29];
            dvp_knode_to(&m_pNodes[19], DVP_ImageConvolution_t)->shiftMask = 6;

            m_pNodes[20].header.kernel = DVP_KN_CONV_5x5;
            dvp_knode_to(&m_pNodes[20], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[20], DVP_ImageConvolution_t)->output = m_images[27];
            dvp_knode_to(&m_pNodes[20], DVP_ImageConvolution_t)->mask = m_images[30];
            dvp_knode_to(&m_pNodes[20], DVP_ImageConvolution_t)->shiftMask = 7;

            m_pNodes[21].header.kernel = DVP_KN_CONV_7x7;
            dvp_knode_to(&m_pNodes[21], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[21], DVP_ImageConvolution_t)->output = m_images[28];
            dvp_knode_to(&m_pNodes[21], DVP_ImageConvolution_t)->mask = m_images[31];
            dvp_knode_to(&m_pNodes[21], DVP_ImageConvolution_t)->shiftMask = 8;

            DVP_Image_Fill(&m_images[29], (DVP_S08*)maskBlurNeg3x3, 3*3*sizeof(DVP_S08));
            DVP_Image_Fill(&m_images[30], (DVP_S08*)maskBlur7x7,    5*5*sizeof(DVP_S08));
            DVP_Image_Fill(&m_images[31], (DVP_S08*)maskBlur7x7,    7*7*sizeof(DVP_S08));

            /*********
              Canny
            **********/
            m_pNodes[22].header.kernel = DVP_KN_CANNY_IMAGE_SMOOTHING;
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_t)->output = m_images[32];
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_t)->mask = m_images[33];
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_t)->shiftMask = 8;

            m_pNodes[23].header.kernel = DVP_KN_CANNY_2D_GRADIENT;
            dvp_knode_to(&m_pNodes[23], DVP_Canny2dGradient_t)->input = m_images[32];
            dvp_knode_to(&m_pNodes[23], DVP_Canny2dGradient_t)->outGradX = m_images[34];
            dvp_knode_to(&m_pNodes[23], DVP_Canny2dGradient_t)->outGradY = m_images[35];
            dvp_knode_to(&m_pNodes[23], DVP_Canny2dGradient_t)->outMag = m_images[36];

            m_pNodes[24].header.kernel = DVP_KN_CANNY_NONMAX_SUPPRESSION;
            dvp_knode_to(&m_pNodes[24], DVP_CannyNonMaxSuppression_t)->inGradX = m_images[34];
            dvp_knode_to(&m_pNodes[24], DVP_CannyNonMaxSuppression_t)->inGradY = m_images[35];
            dvp_knode_to(&m_pNodes[24], DVP_CannyNonMaxSuppression_t)->inMag = m_images[36];
            dvp_knode_to(&m_pNodes[24], DVP_CannyNonMaxSuppression_t)->output = m_images[37];

            m_pNodes[25].header.kernel = DVP_KN_CANNY_HYST_THRESHHOLD;
            dvp_knode_to(&m_pNodes[25], DVP_CannyHystThresholding_t)->inMag = m_images[36];
            dvp_knode_to(&m_pNodes[25], DVP_CannyHystThresholding_t)->inEdgeMap = m_images[37];
            dvp_knode_to(&m_pNodes[25], DVP_CannyHystThresholding_t)->output = m_images[38];
            dvp_knode_to(&m_pNodes[25], DVP_CannyHystThresholding_t)->loThresh = 35;
            dvp_knode_to(&m_pNodes[25], DVP_CannyHystThresholding_t)->hiThresh = 122;

            m_pNodes[26].header.kernel = DVP_KN_INTEGRAL_IMAGE_8;
            dvp_knode_to(&m_pNodes[26], DVP_Transform_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[26], DVP_Transform_t)->output = m_images[39];
            if(dvp_knode_to(&m_pNodes[26], DVP_Transform_t)->input.width> 2049 ||dvp_knode_to(&m_pNodes[26], DVP_Transform_t)->input.height> 819 )
               m_pNodes[26].header.affinity = DVP_CORE_CPU;

            m_pNodes[27].header.kernel = DVP_KN_NOOP;

            DVP_Image_Fill(&m_images[33], (DVP_S08*)gaussian_7x7, 7*7*sizeof(DVP_S08)); // Copying the mask
#if defined(DVP_USE_VRUN)
            /*********
              New Requests
            **********/
            m_pNodes[28].header.kernel = DVP_KN_VRUN_CONV_MxN;
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->output = m_images[41];
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->mask = m_images[43];
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->shiftMask = 4;

            m_pNodes[29].header.kernel = DVP_KN_VRUN_CONV_MxN;
            dvp_knode_to(&m_pNodes[29], DVP_ImageConvolution_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[29], DVP_ImageConvolution_t)->output = m_images[42];
            dvp_knode_to(&m_pNodes[29], DVP_ImageConvolution_t)->mask = m_images[44];
            dvp_knode_to(&m_pNodes[29], DVP_ImageConvolution_t)->shiftMask = 4;
#endif
            DVP_Image_Fill(&m_images[43], (DVP_S08*)maskAvg, 7*sizeof(DVP_S08)); // Copying the mask
            DVP_Image_Fill(&m_images[44], (DVP_S08*)maskAvg, 7*sizeof(DVP_S08)); // Copying the mask

            // Copy (copy full image to full image)
            m_pNodes[30].header.kernel = DVP_KN_COPY;
            dvp_knode_to(&m_pNodes[30], DVP_Transform_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[30], DVP_Transform_t)->output = m_images[45];

            // Pad (copy full image to an offset within a larger buffer)
            m_pNodes[31].header.kernel = DVP_KN_COPY;
            dvp_knode_to(&m_pNodes[31], DVP_Transform_t)->input = m_images[8];
            dvp_knode_to(&m_pNodes[31], DVP_Transform_t)->output = m_images[46];
            DVP_Image_SetPatch(&dvp_knode_to(&m_pNodes[31], DVP_Transform_t)->output,
                m_width/2-15, m_height/2-15, m_width, m_height);

            // Crop (copy part of an image to a full buffer)
            m_pNodes[32].header.kernel = DVP_KN_COPY;
            dvp_knode_to(&m_pNodes[32], DVP_Transform_t)->input = m_images[8];
            DVP_Image_SetPatch(&dvp_knode_to(&m_pNodes[32], DVP_Transform_t)->input,
                m_width/2-15, m_height/2-15, m_width/2-15, m_height/2-15);
            dvp_knode_to(&m_pNodes[32], DVP_Transform_t)->output = m_images[47];

            // Crop + Pad (copy part of an image to part of another image with an offset)
            m_pNodes[33].header.kernel = DVP_KN_COPY;
            dvp_knode_to(&m_pNodes[33], DVP_Transform_t)->input = m_images[8];
            DVP_Image_SetPatch(&dvp_knode_to(&m_pNodes[33], DVP_Transform_t)->input,
                m_width/2-15, m_height/2-15, m_width/2-15, m_height/2-15);

            dvp_knode_to(&m_pNodes[33], DVP_Transform_t)->output = m_images[48];
            DVP_Image_SetPatch(&dvp_knode_to(&m_pNodes[33], DVP_Transform_t)->output,
                m_width/4-15, m_height/4-15, m_width/2-15, m_height/2-15);

            m_pNodes[34].header.kernel = DVP_KN_YUV444p_TO_RGBp;
            dvp_knode_to(&m_pNodes[34], DVP_Transform_t)->input  = m_images[1];
            dvp_knode_to(&m_pNodes[34], DVP_Transform_t)->output = m_images[53];

            /*********
              Imglib
            **********/
            m_pNodes[35].header.kernel = DVP_KN_XSTRIDE_SHIFT;
            dvp_knode_to(&m_pNodes[35], DVP_Transform_t)->input = m_images[8];        // Use 8bit luma
            dvp_knode_to(&m_pNodes[35], DVP_Transform_t)->output = m_images[54];       // Output 16bit luma

            m_pNodes[36].header.kernel = DVP_KN_THR_GT2MAX_8;
            dvp_knode_to(&m_pNodes[36], DVP_Threshold_t)->input = m_images[8];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[36], DVP_Threshold_t)->output = m_images[55];
            dvp_knode_to(&m_pNodes[36], DVP_Threshold_t)->thresh = 128;

            m_pNodes[37].header.kernel = DVP_KN_THR_GT2THR_8;
            dvp_knode_to(&m_pNodes[37], DVP_Threshold_t)->input = m_images[8];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[37], DVP_Threshold_t)->output = m_images[56];
            dvp_knode_to(&m_pNodes[37], DVP_Threshold_t)->thresh = 128;

            m_pNodes[38].header.kernel = DVP_KN_THR_LE2MIN_8;
            dvp_knode_to(&m_pNodes[38], DVP_Threshold_t)->input = m_images[8];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[38], DVP_Threshold_t)->output = m_images[57];
            dvp_knode_to(&m_pNodes[38], DVP_Threshold_t)->thresh = 128;

            m_pNodes[39].header.kernel = DVP_KN_THR_LE2THR_8;
            dvp_knode_to(&m_pNodes[39], DVP_Threshold_t)->input = m_images[8];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[39], DVP_Threshold_t)->output = m_images[58];
            dvp_knode_to(&m_pNodes[39], DVP_Threshold_t)->thresh = 128;

            m_pNodes[40].header.kernel = DVP_KN_THR_GT2MAX_16;
            dvp_knode_to(&m_pNodes[40], DVP_Threshold_t)->input = m_images[54];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[40], DVP_Threshold_t)->output = m_images[59];
            dvp_knode_to(&m_pNodes[40], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;

            m_pNodes[41].header.kernel = DVP_KN_THR_GT2THR_16;
            dvp_knode_to(&m_pNodes[41], DVP_Threshold_t)->input = m_images[54];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[41], DVP_Threshold_t)->output = m_images[60];
            dvp_knode_to(&m_pNodes[41], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;

            m_pNodes[42].header.kernel = DVP_KN_THR_LE2MIN_16;
            dvp_knode_to(&m_pNodes[42], DVP_Threshold_t)->input = m_images[54];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[42], DVP_Threshold_t)->output = m_images[61];
            dvp_knode_to(&m_pNodes[42], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;

            m_pNodes[43].header.kernel = DVP_KN_THR_LE2THR_16;
            dvp_knode_to(&m_pNodes[43], DVP_Threshold_t)->input = m_images[54];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[43], DVP_Threshold_t)->output = m_images[62];
            dvp_knode_to(&m_pNodes[43], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;

            m_pNodes[44].header.kernel = DVP_KN_SOBEL_3x3_8s;
            dvp_knode_to(&m_pNodes[44], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[44], DVP_Transform_t)->output = m_images[63];

            m_pNodes[45].header.kernel = DVP_KN_SOBEL_3x3_8;
            dvp_knode_to(&m_pNodes[45], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[45], DVP_Transform_t)->output = m_images[64];

            m_pNodes[46].header.kernel = DVP_KN_SOBEL_5x5_8s;
            dvp_knode_to(&m_pNodes[46], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[46], DVP_Transform_t)->output = m_images[65];

            m_pNodes[47].header.kernel = DVP_KN_SOBEL_5x5_8;
            dvp_knode_to(&m_pNodes[47], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[47], DVP_Transform_t)->output = m_images[66];

            m_pNodes[48].header.kernel = DVP_KN_SOBEL_7x7_8s;
            dvp_knode_to(&m_pNodes[48], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[48], DVP_Transform_t)->output = m_images[67];

            m_pNodes[49].header.kernel = DVP_KN_SOBEL_7x7_8;
            dvp_knode_to(&m_pNodes[49], DVP_Transform_t)->input = m_images[8];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[49], DVP_Transform_t)->output = m_images[68];

            m_pNodes[50].header.kernel = DVP_KN_SOBEL_3x3_16s;
            dvp_knode_to(&m_pNodes[50], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[50], DVP_Transform_t)->output = m_images[2];

            m_pNodes[51].header.kernel = DVP_KN_SOBEL_3x3_16;
            dvp_knode_to(&m_pNodes[51], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[51], DVP_Transform_t)->output = m_images[3];

            m_pNodes[52].header.kernel = DVP_KN_SOBEL_5x5_16s;
            dvp_knode_to(&m_pNodes[52], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[52], DVP_Transform_t)->output = m_images[4];

            m_pNodes[53].header.kernel = DVP_KN_SOBEL_5x5_16;
            dvp_knode_to(&m_pNodes[53], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[53], DVP_Transform_t)->output = m_images[5];

            m_pNodes[54].header.kernel = DVP_KN_SOBEL_7x7_16s;
            dvp_knode_to(&m_pNodes[54], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[54], DVP_Transform_t)->output = m_images[6];

            m_pNodes[55].header.kernel = DVP_KN_SOBEL_7x7_16;
            dvp_knode_to(&m_pNodes[55], DVP_Transform_t)->input = m_images[54];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[55], DVP_Transform_t)->output = m_images[7];

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }

        if(m_testcore != DVP_CORE_MIN) {
            for(uint32_t testnode=0; testnode<m_numNodes; testnode++) {
                m_pNodes[testnode].header.affinity = m_testcore;
            }
        }

        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(61))
            {
                /** Misc**/
                ImageDebug_Init(&m_imgdbg[0],  &m_images[0],  m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1],  &m_images[1],  m_imgdbg_path, "01_YUV444");

                // images 2-7 moved to end of list

                /** **/
                ImageDebug_Init(&m_imgdbg[8],  &m_images[8],  m_imgdbg_path, "08_luma");
                ImageDebug_Init(&m_imgdbg[9],  &m_images[9],  m_imgdbg_path, "09_RGBpl");
                ImageDebug_Init(&m_imgdbg[10], &m_images[10], m_imgdbg_path, "10_IIRH");
                ImageDebug_Init(&m_imgdbg[11], &m_images[12], m_imgdbg_path, "11_IIRV");
                ImageDebug_Init(&m_imgdbg[12], &m_images[13], m_imgdbg_path, "12_YUV420");
                /** Morph**/
                ImageDebug_Init(&m_imgdbg[13], &m_images[14], m_imgdbg_path, "13_bitimgThresh");
                ImageDebug_Init(&m_imgdbg[14], &m_images[15], m_imgdbg_path, "14_bitimgErodeX");
                ImageDebug_Init(&m_imgdbg[15], &m_images[16], m_imgdbg_path, "15_bitimgErodeSq");
                ImageDebug_Init(&m_imgdbg[16], &m_images[17], m_imgdbg_path, "16_bitimgErodeMsk");
                ImageDebug_Init(&m_imgdbg[17], &m_images[18], m_imgdbg_path, "17_bitimgDilateX");
                ImageDebug_Init(&m_imgdbg[18], &m_images[19], m_imgdbg_path, "18_bitimgDilateSq");
                ImageDebug_Init(&m_imgdbg[19], &m_images[20], m_imgdbg_path, "19_bitimgDilateMsk");
                /** NonMax**/
                ImageDebug_Init(&m_imgdbg[20], &m_images[22], m_imgdbg_path, "20_16bitStride");
                ImageDebug_Init(&m_imgdbg[21], &m_images[23], m_imgdbg_path, "21_nonma3");
                ImageDebug_Init(&m_imgdbg[22], &m_images[24], m_imgdbg_path, "22_nonma5");
                ImageDebug_Init(&m_imgdbg[23], &m_images[25], m_imgdbg_path, "23_nonma7");
                ImageDebug_Init(&m_imgdbg[24], &m_images[26], m_imgdbg_path, "24_imgconv3");
                ImageDebug_Init(&m_imgdbg[25], &m_images[27], m_imgdbg_path, "25_imgconv5");
                ImageDebug_Init(&m_imgdbg[26], &m_images[28], m_imgdbg_path, "26_imgconv7");
                /** Canny**/
                ImageDebug_Init(&m_imgdbg[27], &m_images[32], m_imgdbg_path, "27_cannySmooth");
                ImageDebug_Init(&m_imgdbg[28], &m_images[34], m_imgdbg_path, "28_canny2dgradX");
                ImageDebug_Init(&m_imgdbg[29], &m_images[35], m_imgdbg_path, "29_canny2dgradY");
                ImageDebug_Init(&m_imgdbg[30], &m_images[36], m_imgdbg_path, "30_canny2dmag");
                ImageDebug_Init(&m_imgdbg[31], &m_images[37], m_imgdbg_path, "31_cannyNonma");
                ImageDebug_Init(&m_imgdbg[32], &m_images[38], m_imgdbg_path, "32_dThresh");
                ImageDebug_Init(&m_imgdbg[33], &m_images[39], m_imgdbg_path, "33_intImg");
                ImageDebug_Init(&m_imgdbg[34], &m_images[40], m_imgdbg_path, "34_resv1");
                /** New Requests**/
                ImageDebug_Init(&m_imgdbg[35], &m_images[41], m_imgdbg_path, "35_fir7X1");
                ImageDebug_Init(&m_imgdbg[36], &m_images[42], m_imgdbg_path, "36_fir1X7");
                ImageDebug_Init(&m_imgdbg[37], &m_images[45], m_imgdbg_path, "37_copy");
                ImageDebug_Init(&m_imgdbg[38], &m_images[46], m_imgdbg_path, "38_pad");
                ImageDebug_Init(&m_imgdbg[39], &m_images[47], m_imgdbg_path, "39_crop");
                ImageDebug_Init(&m_imgdbg[40], &m_images[48], m_imgdbg_path, "40_crop_pad");
                ImageDebug_Init(&m_imgdbg[41], &m_images[49], m_imgdbg_path, "41_resv1");
                ImageDebug_Init(&m_imgdbg[42], &m_images[50], m_imgdbg_path, "42_resv1");
                ImageDebug_Init(&m_imgdbg[43], &m_images[51], m_imgdbg_path, "43_resv1");
                ImageDebug_Init(&m_imgdbg[44], &m_images[52], m_imgdbg_path, "44_resv1");
                ImageDebug_Init(&m_imgdbg[45], &m_images[53], m_imgdbg_path, "45_RGBpl");
                /** Imglib **/
                ImageDebug_Init(&m_imgdbg[46], &m_images[54], m_imgdbg_path, "46_luma16");
                ImageDebug_Init(&m_imgdbg[47], &m_images[55], m_imgdbg_path, "47_thr8_gtMAX");
                ImageDebug_Init(&m_imgdbg[48], &m_images[56], m_imgdbg_path, "48_thr8_gtTHR");
                ImageDebug_Init(&m_imgdbg[49], &m_images[57], m_imgdbg_path, "49_thr8_leMIN");
                ImageDebug_Init(&m_imgdbg[50], &m_images[58], m_imgdbg_path, "50_thr8_leTHR");
                ImageDebug_Init(&m_imgdbg[51], &m_images[59], m_imgdbg_path, "51_thr16_gtMAX");
                ImageDebug_Init(&m_imgdbg[52], &m_images[60], m_imgdbg_path, "52_thr16_gtTHR");
                ImageDebug_Init(&m_imgdbg[53], &m_images[61], m_imgdbg_path, "53_thr16_leMIN");
                ImageDebug_Init(&m_imgdbg[54], &m_images[62], m_imgdbg_path, "54_thr16_leTHR");
                ImageDebug_Init(&m_imgdbg[55], &m_images[63], m_imgdbg_path, "55_sobel8_3X3_s");
                ImageDebug_Init(&m_imgdbg[56], &m_images[64], m_imgdbg_path, "56_sobel8_3X3");
                ImageDebug_Init(&m_imgdbg[57], &m_images[65], m_imgdbg_path, "57_sobel8_5X5_s");
                ImageDebug_Init(&m_imgdbg[58], &m_images[66], m_imgdbg_path, "58_sobel8_5X5");
                ImageDebug_Init(&m_imgdbg[59], &m_images[67], m_imgdbg_path, "59_sobel8_7X7_s");
                ImageDebug_Init(&m_imgdbg[60], &m_images[68], m_imgdbg_path, "60_sobel8_7X7");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "61_sobel16_3X3_s");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "62_sobel16_3X3");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "63_sobel16_5X5_s");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "64_sobel16_5X5");
                ImageDebug_Init(&m_imgdbg[6], &m_images[7], m_imgdbg_path, "65_sobel16_7X7_s");
                ImageDebug_Init(&m_imgdbg[7], &m_images[6], m_imgdbg_path, "66_sobel16_7X7");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}


status_e TestVisionEngine::Test_HistGraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VLIB)
    DVP_U08 numBins = 8; // Must be a multiple of 2
    DVP_U08 edgeMap8[8] = {0,32,64,96,128,160,192,224};   // bin[7] only accumulates values of 224, 225-255 are ignored
    DVP_U16 edgeMap16[8] = {0,32,64,96,128,160,192,224}; // bin[7] only accumulates values of 224, 225-255 are ignored
    DVP_U32 i = 0;

    if (m_hDVP)
    {
        if (AllocateImageStructs(3))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_UYVY);
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[1], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[2], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
            }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

       if (AllocateBufferStructs(12))
        {
            DVP_Buffer_Init(&m_buffers[0], 1, numBins);  // edges (8bit)
            DVP_Buffer_Init(&m_buffers[1], 1, 256);         // histArray (8bit)
            DVP_Buffer_Init(&m_buffers[2], 2, numBins);  // h[0]
            DVP_Buffer_Init(&m_buffers[3], 2, numBins);  // h[1]
            DVP_Buffer_Init(&m_buffers[4], 2, numBins);  // h[2]
            DVP_Buffer_Init(&m_buffers[5], 2, numBins);  // hOut (8bit)
            DVP_Buffer_Init(&m_buffers[6], 2, m_width*m_height);  // binWeights
            DVP_Buffer_Init(&m_buffers[7], 2, numBins);  // hout (8bit weighted)
            DVP_Buffer_Init(&m_buffers[8], 2, numBins);  // edges (16bit)
            DVP_Buffer_Init(&m_buffers[9], 2, 256);         // histArray (16bit)
            DVP_Buffer_Init(&m_buffers[10], 2, numBins);  // hOut (16bit)
            DVP_Buffer_Init(&m_buffers[11], 2, numBins);  // hOut (16bit weighted)
            if (!DVP_Buffer_Alloc(m_hDVP, &m_buffers[0], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[1], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[2], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[3], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[4], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[5], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[6], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[7], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[8], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[9], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[10], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[11], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(6) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;      // Get Luma first
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];
            m_pNodes[0].header.affinity = DVP_CORE_DSP;

            m_pNodes[1].header.kernel = DVP_KN_XSTRIDE_CONVERT;    // Generates a 16bit per luma value (still only uses lower 8 bits)
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[2];

            memcpy(m_buffers[0].pData, edgeMap8, sizeof(edgeMap8));      // initialize edges array
            memset(m_buffers[1].pData, 0, 256);                                    // initialize histArray

            m_pNodes[2].header.kernel = DVP_KN_VLIB_HISTOGRAM_8;
            dvp_knode_to(&m_pNodes[2], DVP_Histogram_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[2], DVP_Histogram_t)->edges = m_buffers[0];          //delineates boundaries between bins
            dvp_knode_to(&m_pNodes[2], DVP_Histogram_t)->histArray = m_buffers[1];     //internal buffer should be cleared first time only
            dvp_knode_to(&m_pNodes[2], DVP_Histogram_t)->h[0] = m_buffers[2];
            dvp_knode_to(&m_pNodes[2], DVP_Histogram_t)->h[1] = m_buffers[3];
            dvp_knode_to(&m_pNodes[2], DVP_Histogram_t)->h[2]  = m_buffers[4];
            dvp_knode_to(&m_pNodes[2], DVP_Histogram_t)->hOut  = m_buffers[5];
            dvp_knode_to(&m_pNodes[2], DVP_Histogram_t)->numBins = numBins;
            dvp_knode_to(&m_pNodes[2], DVP_Histogram_t)->binWeight = 1;
            dvp_knode_to(&m_pNodes[2], DVP_Histogram_t)->clearFlag = 1;
           // m_pNodes[2].header.affinity = DVP_CORE_CPU;

            // set all weights to 1
            DVP_U16 *pTmp =  (DVP_U16 *)m_buffers[6].pData;
            for(i=0; i<m_width*m_height; i++)
                pTmp[i] = 1;

            m_pNodes[3].header.kernel = DVP_KN_VLIB_WEIGHTED_HISTOGRAM_8;
            dvp_knode_to(&m_pNodes[3], DVP_Histogram_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[3], DVP_Histogram_t)->edges = m_buffers[0];         //delineates boundaries between bins
            dvp_knode_to(&m_pNodes[3], DVP_Histogram_t)->histArray = m_buffers[1];     //internal buffer should be cleared first time only
            dvp_knode_to(&m_pNodes[3], DVP_Histogram_t)->h[0] = m_buffers[2];
            dvp_knode_to(&m_pNodes[3], DVP_Histogram_t)->h[1] = m_buffers[3];
            dvp_knode_to(&m_pNodes[3], DVP_Histogram_t)->h[2]  = m_buffers[4];
            dvp_knode_to(&m_pNodes[3], DVP_Histogram_t)->hOut  = m_buffers[7];
            dvp_knode_to(&m_pNodes[3], DVP_Histogram_t)->numBins = numBins;
            dvp_knode_to(&m_pNodes[3], DVP_Histogram_t)->binWeights = m_buffers[6];
            dvp_knode_to(&m_pNodes[3], DVP_Histogram_t)->clearFlag = 1;
            m_pNodes[3].header.resv[0] = 1;    // I am explicitly setting this flag since the previous histogram 8 should have already initialized the histArray.
            // m_pNodes[3].header.affinity = DVP_CORE_CPU;

            memcpy(m_buffers[8].pData, edgeMap16, sizeof(edgeMap16));      // initialize edges array
            memset(m_buffers[9].pData, 0, 256*2);                          // initialize histArray

            m_pNodes[4].header.kernel = DVP_KN_VLIB_HISTOGRAM_16;
            dvp_knode_to(&m_pNodes[4], DVP_Histogram_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[4], DVP_Histogram_t)->edges = m_buffers[8];         //delineates boundaries between bins
            dvp_knode_to(&m_pNodes[4], DVP_Histogram_t)->histArray = m_buffers[9];     //internal buffer should be cleared first time only
            dvp_knode_to(&m_pNodes[4], DVP_Histogram_t)->h[0] = m_buffers[2];
            dvp_knode_to(&m_pNodes[4], DVP_Histogram_t)->hOut  = m_buffers[10];
            dvp_knode_to(&m_pNodes[4], DVP_Histogram_t)->numBins = numBins;
            dvp_knode_to(&m_pNodes[4], DVP_Histogram_t)->binWeight = 1;
            dvp_knode_to(&m_pNodes[4], DVP_Histogram_t)->clearFlag = 1;
            //m_pNodes[4].header.affinity = DVP_CORE_CPU;

            m_pNodes[5].header.kernel = DVP_KN_VLIB_WEIGHTED_HISTOGRAM_16;
            dvp_knode_to(&m_pNodes[5], DVP_Histogram_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[5], DVP_Histogram_t)->edges = m_buffers[8];         //delineates boundaries between bins
            dvp_knode_to(&m_pNodes[5], DVP_Histogram_t)->histArray = m_buffers[9];     //internal buffer should be cleared first time only
            dvp_knode_to(&m_pNodes[5], DVP_Histogram_t)->h[0] = m_buffers[2];
            dvp_knode_to(&m_pNodes[5], DVP_Histogram_t)->hOut  = m_buffers[11];
            dvp_knode_to(&m_pNodes[5], DVP_Histogram_t)->numBins = numBins;
            dvp_knode_to(&m_pNodes[5], DVP_Histogram_t)->binWeights = m_buffers[6];
            dvp_knode_to(&m_pNodes[5], DVP_Histogram_t)->clearFlag = 1;
            m_pNodes[5].header.resv[0] = 1;    // I am explicitly setting this flag since the previous histogram 16 should have already initialized the histArray.
            // m_pNodes[5].header.affinity = DVP_CORE_CPU;

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;

        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(7))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_luma16bit");
                BufferDebug_Init(&m_imgdbg[3], &m_buffers[5], m_imgdbg_path, "03_8bitHist", ".bw");
                BufferDebug_Init(&m_imgdbg[4], &m_buffers[7], m_imgdbg_path, "04_8bitWHist", ".bw");
                BufferDebug_Init(&m_imgdbg[5], &m_buffers[10], m_imgdbg_path, "05_16bitHist", ".bw");
                BufferDebug_Init(&m_imgdbg[6], &m_buffers[11], m_imgdbg_path, "06_16bitWHist", ".bw");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
        return status;
}

status_e TestVisionEngine::Test_TeslaGraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VLIB)
    DVP_U32 i;
    DVP_Bounds_t bound = {NULL, 0, 0};
    DVP_S32 ONE_10p5 = 32, ONE_8p7 = 128;

#define CAMERA_NOISE_S16    0x0A00      //SQ12.3
#define CAMERA_NOISE_S32      0x00A00000    // SQ16.15
#define THRESHOLD_FACTOR_S16    0x31ff      //SQ4.11
#define THRESHOLD_FACTOR_S32  0x31fffff9    // SQ4.27
#define MAX_S16 32767
#define MAX_0p16  1
#define MAX_4p12  16
#define MAX_12p4  4096
#define MAX_13p2  8192
#define MAX_10p5  1024
#define NFEATURES   4
#define IIR_ALPHA_S16         0x10          // SQ0.15
#define IIR_ALPHA_S32         0x00001000    // SQ0.31
#define LGM_ORDER   6
#define LAB_D   4
#define sD_2x4 (4)
#define mD_2x4 (2)
#define sD_4x6 (6)
#define mD_4x6 (2)
#define SimplexN    3
#define MAX_S32_S 2147483647
#define MAX_4p27 16
#define HIST_ND_DIMX    4
    // DECLARE AND INITIALIZE MIXTURE OF GAUSSIANS PARAMETERS
    // update rate for weights, SQ0.15
    DVP_S16 updateRate1 = 0.001 * MAX_S16/MAX_0p16;
    // update rate for means and variances, SQ0.31
    DVP_S16 updateRate2 = 0.001 * MAX_S16/MAX_0p16;
    // mahalanobis distance threshold, SQ4.27
    DVP_S16 mdThreshold = (2.5*2.5) * (MAX_S16/MAX_4p12);
    // background subtraction threshold, SQ0.15
    DVP_S16 bsThreshold = 0.9 * MAX_S16/MAX_0p16;
    // initial weight for new component, SQ0.15
    DVP_S16 initialWt   = 0.001 * (MAX_S16/MAX_0p16);
    // initial variance for new component, SQ16.15
    DVP_S16 initialVar  = 320 * (MAX_S16/MAX_12p4);

    if (m_hDVP)
    {
        if (AllocateImageStructs(65))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_Y16); //HSLp (H 16bit)
            DVP_Image_Init(&m_images[5], m_width-4, m_height, FOURCC_Y800);
            if((m_testcore == DVP_CORE_DSP) || (m_testcore == DVP_CORE_CPU))
            {
                DVP_Image_Init(&m_images[6], (m_width-4)/2, (m_height-3)/2, FOURCC_Y800);
                DVP_Image_Init(&m_images[7], (m_width-4)/2, (m_height-3)/2, FOURCC_Y800);
            }
            else
            {
                DVP_Image_Init(&m_images[6], (m_width)/2, (m_height)/2, FOURCC_Y800);
                DVP_Image_Init(&m_images[7], (m_width)/2, (m_height)/2, FOURCC_Y800);
            }
            DVP_Image_Init(&m_images[8], (m_width/8), (m_height/8)*21, FOURCC_Y800);
            DVP_Image_Init(&m_images[9], (m_width/8), (m_height/8)*21, FOURCC_Y16);
            DVP_Image_Init(&m_images[10], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[11], m_width, m_height, FOURCC_RGBA);
            DVP_Image_Init(&m_images[12], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[13], m_width, m_height, FOURCC_RGBA);
            DVP_Image_Init(&m_images[14], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[15], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[16], m_width, 8, FOURCC_Y16);
            DVP_Image_Init(&m_images[17], m_width/32, m_height, FOURCC_RGBA);
            DVP_Image_Init(&m_images[18], m_width/32, m_height, FOURCC_RGBA);
            DVP_Image_Init(&m_images[19], m_width, m_height*3, FOURCC_Y16); //mog wts
            DVP_Image_Init(&m_images[20], m_width, m_height*3, FOURCC_Y16); //mog mu
            DVP_Image_Init(&m_images[21], m_width, m_height*3, FOURCC_Y16); //mog var
            DVP_Image_Init(&m_images[22], m_width, m_height*3, FOURCC_Y800); //mog intbuf for 16/32
            DVP_Image_Init(&m_images[23], m_width, m_height, FOURCC_Y800); //mog compidx
            DVP_Image_Init(&m_images[24], m_width/32, m_height, FOURCC_RGBA); //mog fgmask
            DVP_Image_Init(&m_images[25], m_width, m_height*3, FOURCC_Y16); //mog wts
            DVP_Image_Init(&m_images[26], m_width, m_height*3, FOURCC_RGBA); //mog mu
            DVP_Image_Init(&m_images[27], m_width, m_height*3, FOURCC_RGBA); //mog var
            DVP_Image_Init(&m_images[28], m_width, m_height, FOURCC_Y800); //mog compidx
            DVP_Image_Init(&m_images[29], m_width/32, m_height, FOURCC_RGBA); //mog fgmask
            DVP_Image_Init(&m_images[30], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[31], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[32], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[33], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[34], m_width, m_height, FOURCC_YV16);
            DVP_Image_Init(&m_images[35], m_width, m_height, FOURCC_IYUV);
            DVP_Image_Init(&m_images[36], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[37], m_width, m_height, FOURCC_Y16);  // L (LABp)
            DVP_Image_Init(&m_images[38], m_width, m_height, FOURCC_Y16);  // a (LABp)
            DVP_Image_Init(&m_images[39], m_width, m_height, FOURCC_Y16);  // b (LABp)
            DVP_Image_Init(&m_images[40], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[41], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[42], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[43], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[44], 267, 267, FOURCC_Y16);            //hough space
            DVP_Image_Init(&m_images[45], m_width/32, m_height, FOURCC_Y800);   //mask for EWR
            DVP_Image_Init(&m_images[46], m_width/32, m_height, FOURCC_Y800);   //mask for EWR
            DVP_Image_Init(&m_images[47], m_width/32, m_height, FOURCC_Y800);   //mask for EWR
            DVP_Image_Init(&m_images[48], m_width/32, m_height, FOURCC_Y800);   //mask for EWR
            DVP_Image_Init(&m_images[49], m_width/32, m_height, FOURCC_Y800);   //mask for UWR
            DVP_Image_Init(&m_images[50], m_width/32, m_height, FOURCC_Y800);   //mask for UWR
            DVP_Image_Init(&m_images[51], m_width, m_height, FOURCC_Y800); //HSLp (S)
            DVP_Image_Init(&m_images[52], m_width, m_height, FOURCC_Y800); //HSLp (L)
            DVP_Image_Init(&m_images[53], m_width, m_height, FOURCC_NV12);
            DVP_Image_Init(&m_images[54], m_width, m_height, FOURCC_YV24);
            DVP_Image_Init(&m_images[55], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[56], m_width, m_height, FOURCC_RGBA); //hyst thresh scratch
            DVP_Image_Init(&m_images[57], m_width, m_height+1, FOURCC_RGBA); //intimg16
            DVP_Image_Init(&m_images[58], m_width, m_height, FOURCC_Y800);//extractBack16
            if((m_testcore == DVP_CORE_DSP) || (m_testcore == DVP_CORE_CPU))
                DVP_Image_Init(&m_images[59], (m_width-4)/2, (m_height-4)/2, FOURCC_Y800);
            else
                DVP_Image_Init(&m_images[59], (m_width)/2, (m_height)/2, FOURCC_Y800);
            DVP_Image_Init(&m_images[60], (m_width-4)/2, (m_height-4)/2, FOURCC_Y16);
            DVP_Image_Init(&m_images[61], m_width-4, 5, FOURCC_RGBA);//scratch buffer for gauss5x5
            DVP_Image_Init(&m_images[62], m_width/8, m_height, FOURCC_Y800); //pack
            DVP_Image_Init(&m_images[63], m_width, m_height, FOURCC_Y800); //unpack
            DVP_Image_Init(&m_images[64], m_width, m_height, FOURCC_RGBA); //harris32
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[1], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[2], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[3], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[4], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[5], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[6], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[7], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[8], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[9], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[10], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[11], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[12], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[13], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[14], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[15], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[16], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[17], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[18], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[19], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[20], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[21], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[22], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[23], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[24], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[25], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[26], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[27], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[28], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[29], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[30], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[31], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[32], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[33], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[34], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[35], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[36], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[37], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[38], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[39], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[40], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[41], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[42], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[43], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[44], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[45], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[46], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[47], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[48], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[49], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[50], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[51], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[52], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[53], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[54], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[55], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[56], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[57], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[58], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[59], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[60], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[61], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[62], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[63], DVP_MTYPE_DEFAULT) ||
                 !DVP_Image_Alloc(m_hDVP, &m_images[64], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
            }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateBufferStructs(67))
        {
            DVP_Buffer_Init(&m_buffers[0], 1, 192*m_width);  // scratch harris
            DVP_Buffer_Init(&m_buffers[1], 2, m_width*m_height*.05); //5% edge points
            DVP_Buffer_Init(&m_buffers[2], 2, 267); //Sin
            DVP_Buffer_Init(&m_buffers[3], 2, 267); //Cos
            DVP_Buffer_Init(&m_buffers[4], 2, 267 + 267%4); //ping
            DVP_Buffer_Init(&m_buffers[5], 2, 267 + 267%4); //pong
            DVP_Buffer_Init(&m_buffers[6], 2, 267 + 267%4); //pang
            DVP_Buffer_Init(&m_buffers[7], 2, 267 + 267%4); //peng
            DVP_Buffer_Init(&m_buffers[8], 2, NFEATURES); //x
            DVP_Buffer_Init(&m_buffers[9], 2, NFEATURES); //y
            DVP_Buffer_Init(&m_buffers[10], 2, NFEATURES); //outx
            DVP_Buffer_Init(&m_buffers[11], 2, NFEATURES); //outx
            DVP_Buffer_Init(&m_buffers[12], 1, 384); //scratch
            DVP_Buffer_Init(&m_buffers[13], 2, (LGM_ORDER+1)*m_height); //legendre poly
            DVP_Buffer_Init(&m_buffers[14], 2, (LGM_ORDER+1)*(LGM_ORDER+1)); //legendre mom
            DVP_Buffer_Init(&m_buffers[15], 2, m_width*m_height); //kalman filter 2x4
            DVP_Buffer_Init(&m_buffers[16], 2, sD_2x4*sD_2x4); //transition
            DVP_Buffer_Init(&m_buffers[17], 2, sD_2x4*sD_2x4); //errorCov
            DVP_Buffer_Init(&m_buffers[18], 2, sD_2x4*sD_2x4); //predictedErrorCov
            DVP_Buffer_Init(&m_buffers[19], 2, sD_2x4*sD_2x4); //processNoiseCov
            DVP_Buffer_Init(&m_buffers[20], 2, mD_2x4*mD_2x4); //measurementNoiseCov
            DVP_Buffer_Init(&m_buffers[21], 2, sD_2x4*mD_2x4); //measurement
            DVP_Buffer_Init(&m_buffers[22], 2, sD_2x4); //state
            DVP_Buffer_Init(&m_buffers[23], 2, sD_2x4); //predictedState
            DVP_Buffer_Init(&m_buffers[24], 2, sD_2x4*mD_2x4); //kalmanGain
            DVP_Buffer_Init(&m_buffers[25], 2, sD_2x4*sD_2x4); //temp1
            DVP_Buffer_Init(&m_buffers[26], 2, sD_2x4*sD_2x4); //temp2
            DVP_Buffer_Init(&m_buffers[27], 2, sD_2x4*sD_2x4); //temp3
            DVP_Buffer_Init(&m_buffers[28], 4, m_width*m_height); //kalman filter 4x6
            DVP_Buffer_Init(&m_buffers[29], 2, sD_4x6*sD_4x6); //transition
            DVP_Buffer_Init(&m_buffers[30], 2, sD_4x6*sD_4x6); //errorCov
            DVP_Buffer_Init(&m_buffers[31], 2, sD_4x6*sD_4x6); //predictedErrorCov
            DVP_Buffer_Init(&m_buffers[32], 2, sD_4x6*sD_4x6); //processNoiseCov
            DVP_Buffer_Init(&m_buffers[33], 2, mD_4x6*mD_4x6); //measurementNoiseCov
            DVP_Buffer_Init(&m_buffers[34], 2, sD_4x6*mD_4x6); //measurement
            DVP_Buffer_Init(&m_buffers[35], 2, sD_4x6); //state
            DVP_Buffer_Init(&m_buffers[36], 2, sD_4x6); //predictedState
            DVP_Buffer_Init(&m_buffers[37], 2, sD_4x6*mD_4x6); //kalmanGain
            DVP_Buffer_Init(&m_buffers[38], 2, sD_4x6*sD_4x6); //temp1
            DVP_Buffer_Init(&m_buffers[39], 2, sD_4x6*sD_4x6); //temp2
            DVP_Buffer_Init(&m_buffers[40], 2, sD_4x6*sD_4x6); //temp3
            DVP_Buffer_Init(&m_buffers[41], 2, SimplexN); //Simplex start
            DVP_Buffer_Init(&m_buffers[42], 2, SimplexN); //Simplex step
            DVP_Buffer_Init(&m_buffers[43], 2, SimplexN); //Simplex stop
            DVP_Buffer_Init(&m_buffers[44], 2, SimplexN+1); //Simplex v
            DVP_Buffer_Init(&m_buffers[45], 2, SimplexN+1); //Simplex f
            DVP_Buffer_Init(&m_buffers[46], 2, SimplexN); //Simplex vr
            DVP_Buffer_Init(&m_buffers[47], 2, SimplexN); //Simplex ve
            DVP_Buffer_Init(&m_buffers[48], 2, SimplexN); //Simplex vc
            DVP_Buffer_Init(&m_buffers[49], 2, SimplexN); //Simplex vm
            DVP_Buffer_Init(&m_buffers[50], 2, SimplexN); //Simplex3D start
            DVP_Buffer_Init(&m_buffers[51], 2, SimplexN); //Simplex3D step
            DVP_Buffer_Init(&m_buffers[52], 2, SimplexN); //Simplex3D stop
            DVP_Buffer_Init(&m_buffers[53], 2, SimplexN+1); //Simplex3D v
            DVP_Buffer_Init(&m_buffers[54], 2, SimplexN+1); //Simplex3D f
            DVP_Buffer_Init(&m_buffers[55], 2, SimplexN); //Simplex3D vr
            DVP_Buffer_Init(&m_buffers[56], 2, SimplexN); //Simplex3D ve
            DVP_Buffer_Init(&m_buffers[57], 2, SimplexN); //Simplex3D vc
            DVP_Buffer_Init(&m_buffers[58], 2, SimplexN); //Simplex3D vm
            DVP_Buffer_Init(&m_buffers[59], 2, HIST_ND_DIMX); //Hist nD16 nbins
            DVP_Buffer_Init(&m_buffers[60], 2, HIST_ND_DIMX); //Hist nD16 norms
            DVP_Buffer_Init(&m_buffers[61], 2, m_width*m_height/HIST_ND_DIMX); //Hist int1
            DVP_Buffer_Init(&m_buffers[62], 2, 32*32*32*32); //Hist nD16 int2
            DVP_Buffer_Init(&m_buffers[63], 2, 32*32*32*32); //Hist nD16 H
            DVP_Buffer_Init(&m_buffers[64], 4, m_width*m_height); //nms 32
            DVP_Buffer_Init(&m_buffers[65], 4, 4*(7 + m_width*m_height)); //nms5x5 32 scratch
            DVP_Buffer_Init(&m_buffers[66], 1, 2197000); //lab LUT ref:VLIB_initUYVYint_to_LABpl_LUT_i.c
            if (!DVP_Buffer_Alloc(m_hDVP, &m_buffers[0], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[1], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[2], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[3], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[4], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[5], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[6], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[7], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[8], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[9], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[10], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[11], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[12], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[13], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[14], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[15], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[16], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[17], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[18], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[19], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[20], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[21], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[22], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[23], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[24], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[25], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[26], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[27], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[28], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[29], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[30], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[31], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[32], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[33], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[34], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[35], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[36], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[37], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[38], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[39], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[40], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[41], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[42], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[43], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[44], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[45], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[46], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[47], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[48], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[49], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[50], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[51], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[52], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[53], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[54], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[55], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[56], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[57], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[58], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[59], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[60], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[61], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[62], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[63], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[64], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[65], DVP_MTYPE_DEFAULT) ||
                !DVP_Buffer_Alloc(m_hDVP, &m_buffers[66], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        /*init Sin and Cos LUT for houghLines
        */
        float Theta;
        float ThetaMin = -80;
        float ThetaMax = 79.6;
        float NumThetaPoints = 267;
        DVP_S16 *pSin = (DVP_S16*)m_buffers[2].pData;
        DVP_S16 *pCos = (DVP_S16*)m_buffers[3].pData;
        for(i = 0; i < NumThetaPoints; i++)
        {

            Theta = ThetaMin + i*(ThetaMax - ThetaMin)/(NumThetaPoints-1);

            *(pSin + i) = (DVP_S16)(floor(256*sin(Theta*0.017453) + 0.5));
            *(pCos + i) = (DVP_S16)(floor(256*cos(Theta*0.017453) + 0.5));

            // pTRIG[i] = (*(pCos + i)) | ((S32)(*(pSin + i)) << 16);;

        }
        /*init Sin and Cos LUT for houghLines
        */

        /* Init buffers for Kalman Filter 2x4 */
        DVP_S16 *tmpPtr;
        memset(m_buffers[16].pData, 0, sD_2x4*sD_2x4*sizeof(DVP_S16));
        memset(m_buffers[17].pData, 0, sD_2x4*sD_2x4*sizeof(DVP_S16));
        memset(m_buffers[18].pData, 0, sD_2x4*sD_2x4*sizeof(DVP_S16));
        memset(m_buffers[19].pData, 0, sD_2x4*sD_2x4*sizeof(DVP_S16));
        memset(m_buffers[20].pData, 0, mD_2x4*mD_2x4*sizeof(DVP_S16));
        memset(m_buffers[21].pData, 0, sD_2x4*mD_2x4*sizeof(DVP_S16));
        memset(m_buffers[22].pData, 0, sD_2x4*sizeof(DVP_S16));
        memset(m_buffers[23].pData, 0, sD_2x4*sizeof(DVP_S16));
        memset(m_buffers[24].pData, 0, sD_2x4*mD_2x4*sizeof(DVP_S16));
        memset(m_buffers[25].pData, 0, sD_2x4*sD_2x4*sizeof(DVP_S16));
        memset(m_buffers[26].pData, 0, sD_2x4*sD_2x4*sizeof(DVP_S16));
        memset(m_buffers[27].pData, 0, sD_2x4*sD_2x4*sizeof(DVP_S16));

        // initialize matrices
        for(i=0;i<sD_2x4;i++){
            tmpPtr = (DVP_S16*)m_buffers[17].pData;
            tmpPtr[(i*sD_2x4) + i] = 1000 * MAX_S16/MAX_13p2;
            tmpPtr = (DVP_S16*)m_buffers[16].pData;
            tmpPtr[(i*sD_2x4) + i] = 1;
            tmpPtr = (DVP_S16*)m_buffers[21].pData;
            if(i < mD_2x4)
                tmpPtr[(i*sD_2x4) + i] = 1;
        }
        tmpPtr = (DVP_S16*)m_buffers[20].pData;
        tmpPtr[0] = 8100;// * MAX_S16 / MAX_13p2;
        tmpPtr[3] = 8100;// * MAX_S16 / MAX_13p2;

        tmpPtr = (DVP_S16*)m_buffers[16].pData;
        tmpPtr[2] = 1;
        tmpPtr[7] = 1;

        tmpPtr = (DVP_S16*)m_buffers[19].pData;
        tmpPtr[0] = 0.333 * MAX_S16 / MAX_13p2;
        tmpPtr[1] = 0.50 * MAX_S16 / MAX_13p2;
        tmpPtr[4] = 0.50 * MAX_S16 / MAX_13p2;
        tmpPtr[5] = 1.0 * MAX_S16 / MAX_13p2;
        tmpPtr[10] = 0.333 * MAX_S16 / MAX_13p2;
        tmpPtr[11] = 0.50 * MAX_S16 / MAX_13p2;
        tmpPtr[14] = 0.50 * MAX_S16 / MAX_13p2;
        tmpPtr[15] = 1.0 * MAX_S16 / MAX_13p2;

        // set initial state to all 0s
        tmpPtr = (DVP_S16*)m_buffers[22].pData;
        tmpPtr[0] = 0 * MAX_S16 / MAX_10p5;
        tmpPtr[1] = 0 * MAX_S16 / MAX_10p5;
        tmpPtr[2] = 0 * MAX_S16 / MAX_10p5;
        tmpPtr[3] = 0 * MAX_S16 / MAX_10p5;
        /* Init buffers for Kalman Filter 2x4 */

        /* Init buffers for Kalman Filter 4x6 */
        memset(m_buffers[29].pData, 0, sD_4x6*sD_4x6*sizeof(DVP_S16));
        memset(m_buffers[30].pData, 0, sD_4x6*sD_4x6*sizeof(DVP_S16));
        memset(m_buffers[31].pData, 0, sD_4x6*sD_4x6*sizeof(DVP_S16));
        memset(m_buffers[32].pData, 0, sD_4x6*sD_4x6*sizeof(DVP_S16));
        memset(m_buffers[33].pData, 0, mD_4x6*mD_4x6*sizeof(DVP_S16));
        memset(m_buffers[34].pData, 0, sD_4x6*mD_4x6*sizeof(DVP_S16));
        memset(m_buffers[35].pData, 0, sD_4x6*sizeof(DVP_S16));
        memset(m_buffers[36].pData, 0, sD_4x6*sizeof(DVP_S16));
        memset(m_buffers[37].pData, 0, sD_4x6*mD_4x6*sizeof(DVP_S16));
        memset(m_buffers[38].pData, 0, sD_4x6*sD_4x6*sizeof(DVP_S16));
        memset(m_buffers[39].pData, 0, sD_4x6*sD_4x6*sizeof(DVP_S16));
        memset(m_buffers[40].pData, 0, sD_4x6*sD_4x6*sizeof(DVP_S16));

        // initialize matrices
        for(i=0;i<sD_4x6;i++){
            tmpPtr = (DVP_S16*)m_buffers[30].pData;
            tmpPtr[(i*sD_4x6) + i] = 1000 * MAX_S16/MAX_13p2;
            tmpPtr = (DVP_S16*)m_buffers[29].pData;
            tmpPtr[(i*sD_4x6) + i] = 1 * MAX_S16/MAX_13p2;
            tmpPtr = (DVP_S16*)m_buffers[34].pData;
            if(i < mD_4x6)
                tmpPtr[(i*sD_4x6) + i] = 1;
        }

        tmpPtr = (DVP_S16*)m_buffers[29].pData;
        tmpPtr[2] = 1 * MAX_S16/MAX_13p2;
        tmpPtr[4] = 0.5 * MAX_S16/MAX_13p2;
        tmpPtr[9] = 1 * MAX_S16/MAX_13p2;
        tmpPtr[11] = 0.5 * MAX_S16/MAX_13p2;
        tmpPtr[16] = 1 * MAX_S16/MAX_13p2;
        tmpPtr[23] = 1 * MAX_S16/MAX_13p2;

        tmpPtr = (DVP_S16*)m_buffers[33].pData;
        tmpPtr[0] = 2500; //2500; //8100;// * MAX_S16 / MAX_13p2;
        tmpPtr[5] = 2500; //2500; //8100;// * MAX_S16 / MAX_13p2;
        tmpPtr[10] = 100;
        tmpPtr[15] = 100;

        tmpPtr = (DVP_S16*)m_buffers[32].pData;
        tmpPtr[0] = 0.25 * MAX_S16/MAX_13p2;
        tmpPtr[1] = 0.25 * MAX_S16/MAX_13p2;
        tmpPtr[2] = 0.25 * MAX_S16/MAX_13p2;
        tmpPtr[6] = 0.25 * MAX_S16/MAX_13p2;
        tmpPtr[7] = 0.2500 * MAX_S16/MAX_13p2;
        tmpPtr[8] = 0.5000 * MAX_S16/MAX_13p2;
        tmpPtr[12] = 0.25 * MAX_S16/MAX_13p2;
        tmpPtr[13] = 0.5000 * MAX_S16/MAX_13p2;
        tmpPtr[14] = 1.000 * MAX_S16/MAX_13p2;

        tmpPtr = (DVP_S16*)m_buffers[32].pData;
        tmpPtr[21] = 0.25 * MAX_S16/MAX_13p2;
        tmpPtr[22] = 0.25 * MAX_S16/MAX_13p2;
        tmpPtr[23] = 0.25 * MAX_S16/MAX_13p2;
        tmpPtr[27] = 0.25 * MAX_S16/MAX_13p2;
        tmpPtr[28] = 0.2500 * MAX_S16/MAX_13p2;
        tmpPtr[29] = 0.5000 * MAX_S16/MAX_13p2;
        tmpPtr[33] = 0.25 * MAX_S16/MAX_13p2;
        tmpPtr[34] = 0.5000 * MAX_S16/MAX_13p2;
        tmpPtr[35] = 1.000 * MAX_S16/MAX_13p2;

        // set initial measurement
        tmpPtr = (DVP_S16*)m_buffers[35].pData;
        tmpPtr[0] = 190 * ONE_10p5;
        tmpPtr[1] = 190 * ONE_10p5;
        tmpPtr[2] = 0 * ONE_8p7;
        tmpPtr[3] = 0 * ONE_8p7;
        tmpPtr[4] = 0 * ONE_8p7;
        tmpPtr[5] = 0 * ONE_8p7;
        /* Init buffers for Kalman Filter 4x6 */

        if (AllocateNodes(54) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;      // Get Luma first
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];
            m_pNodes[0].header.affinity = DVP_CORE_DSP;

            m_pNodes[1].header.kernel = DVP_KN_XSTRIDE_CONVERT;    // Generates a 16bit per luma value (still only uses lower 8 bits) for X
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[2];

            m_pNodes[2].header.kernel = DVP_KN_XSTRIDE_CONVERT;    // Generates 16bit per luma for Y
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->output = m_images[3];

            m_pNodes[3].header.kernel = DVP_KN_VLIB_BHATTACHAYA_DISTANCE;    // Call Battacharya distance on dsp
            dvp_knode_to(&m_pNodes[3], DVP_Distance_t)->X = m_images[2];
            dvp_knode_to(&m_pNodes[3], DVP_Distance_t)->Y = m_images[3];
            dvp_knode_to(&m_pNodes[3], DVP_Distance_t)->distance = 0;             //Initialize to zero
            m_pNodes[3].header.affinity = DVP_CORE_DSP;

            m_pNodes[4].header.kernel = DVP_KN_VLIB_L1DISTANCE;    // Call L1 distance on dsp
            dvp_knode_to(&m_pNodes[4], DVP_Distance_t)->X = m_images[2];
            dvp_knode_to(&m_pNodes[4], DVP_Distance_t)->Y = m_images[3];
            dvp_knode_to(&m_pNodes[4], DVP_Distance_t)->distance = 0;     //Initialize to zero
            m_pNodes[4].header.affinity = DVP_CORE_DSP;

            m_pNodes[5].header.kernel = DVP_KN_VLIB_UYVY_TO_HSLp;
            dvp_knode_to(&m_pNodes[5], DVP_Int2Pl_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[5], DVP_Int2Pl_t)->output1 = m_images[4];
            dvp_knode_to(&m_pNodes[5], DVP_Int2Pl_t)->output2 = m_images[51];
            dvp_knode_to(&m_pNodes[5], DVP_Int2Pl_t)->output3 = m_images[52];
            m_pNodes[5].header.affinity = DVP_CORE_DSP;

            if((m_testcore == DVP_CORE_DSP) || (m_testcore == DVP_CORE_CPU))
            {
                m_pNodes[6].header.kernel = DVP_KN_VLIB_GRADIENT_V5x5_PYRAMID_8;
                dvp_knode_to(&m_pNodes[6], DVP_Gradient_t)->input = m_images[1];
                dvp_knode_to(&m_pNodes[6], DVP_Gradient_t)->output = m_images[6];
                dvp_knode_to(&m_pNodes[6], DVP_Gradient_t)->scratch = m_images[5];
                m_pNodes[6].header.affinity = DVP_CORE_DSP;
            }
#if defined(DVP_USE_VRUN)
            else
            {
                m_pNodes[6].header.kernel = DVP_KN_VRUN_GRADIENT_V5x5_PYRAMID_8;
                dvp_knode_to(&m_pNodes[6], DVP_Transform_t)->input = m_images[1];
                dvp_knode_to(&m_pNodes[6], DVP_Transform_t)->output = m_images[6];
                m_pNodes[6].header.affinity = DVP_CORE_SIMCOP;
            }
#endif
            if((m_testcore == DVP_CORE_DSP) || (m_testcore == DVP_CORE_CPU))
            {
                m_pNodes[7].header.kernel = DVP_KN_VLIB_GRADIENT_H5x5_PYRAMID_8;
                dvp_knode_to(&m_pNodes[7], DVP_Gradient_t)->input = m_images[1];
                dvp_knode_to(&m_pNodes[7], DVP_Gradient_t)->output = m_images[7];
                dvp_knode_to(&m_pNodes[7], DVP_Gradient_t)->scratch = m_images[5];
                m_pNodes[7].header.affinity = DVP_CORE_DSP;
            }
#if defined(DVP_USE_VRUN)
            else
            {
                m_pNodes[7].header.kernel = DVP_KN_VRUN_GRADIENT_H5x5_PYRAMID_8;
                dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->input = m_images[1];
                dvp_knode_to(&m_pNodes[7], DVP_Transform_t)->output = m_images[7];
                m_pNodes[7].header.affinity = DVP_CORE_SIMCOP;
            }
#endif
            if((m_testcore == DVP_CORE_DSP) || (m_testcore == DVP_CORE_CPU))
            {
                m_pNodes[8].header.kernel = DVP_KN_VLIB_IMAGE_PYRAMID_8;
                dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->input = m_images[1];
                dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->output = m_images[8];
                m_pNodes[8].header.affinity = DVP_CORE_DSP;
            }
#if defined(DVP_USE_VRUN)
            else
            {
                m_pNodes[8].header.kernel = DVP_KN_VRUN_IMAGE_PYRAMID_8;
                dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->input = m_images[1];
                dvp_knode_to(&m_pNodes[8], DVP_Transform_t)->output = m_images[8];
                m_pNodes[8].header.affinity = DVP_CORE_SIMCOP;
            }
#endif

            m_pNodes[9].header.kernel = DVP_KN_VLIB_IMAGE_PYRAMID_16;
            dvp_knode_to(&m_pNodes[9], DVP_Transform_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[9], DVP_Transform_t)->output = m_images[9];
            m_pNodes[9].header.affinity = DVP_CORE_DSP;

            m_pNodes[10].header.kernel = DVP_KN_VLIB_INIT_MEAN_LUMA_S16;
            dvp_knode_to(&m_pNodes[10], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[10], DVP_Transform_t)->output = m_images[10];
            m_pNodes[10].header.affinity = DVP_CORE_DSP;

            m_pNodes[11].header.kernel = DVP_KN_VLIB_INIT_MEAN_LUMA_S32;
            dvp_knode_to(&m_pNodes[11], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[11], DVP_Transform_t)->output = m_images[11];
            m_pNodes[11].header.affinity = DVP_CORE_DSP;

            m_pNodes[12].header.kernel = DVP_KN_VLIB_INIT_VAR_CONST_S16;
            dvp_knode_to(&m_pNodes[12], DVP_ConstVar_t)->var = m_images[12];
            dvp_knode_to(&m_pNodes[12], DVP_ConstVar_t)->constVar = CAMERA_NOISE_S16/32;
            m_pNodes[12].header.affinity = DVP_CORE_DSP;

            m_pNodes[13].header.kernel = DVP_KN_VLIB_INIT_VAR_CONST_S32;
            dvp_knode_to(&m_pNodes[13], DVP_ConstVar_t)->var = m_images[13];
            dvp_knode_to(&m_pNodes[13], DVP_ConstVar_t)->constVar = CAMERA_NOISE_S32/32;
            m_pNodes[13].header.affinity = DVP_CORE_DSP;

            m_pNodes[14].header.kernel = DVP_KN_VLIB_IIR_HORZ_16;
            dvp_knode_to(&m_pNodes[14], DVP_IIR_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[14], DVP_IIR_t)->output = m_images[14];
            dvp_knode_to(&m_pNodes[14], DVP_IIR_t)->weight = 2000;
            dvp_knode_to(&m_pNodes[14], DVP_IIR_t)->bounds[0] = bound;
            dvp_knode_to(&m_pNodes[14], DVP_IIR_t)->bounds[1] = bound;
            dvp_knode_to(&m_pNodes[14], DVP_IIR_t)->scratch = m_images[16];
            m_pNodes[14].header.affinity = DVP_CORE_DSP;

            m_pNodes[15].header.kernel = DVP_KN_VLIB_IIR_VERT_16;
            dvp_knode_to(&m_pNodes[15], DVP_IIR_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[15], DVP_IIR_t)->output = m_images[15];
            dvp_knode_to(&m_pNodes[15], DVP_IIR_t)->weight = 2000;
            dvp_knode_to(&m_pNodes[15], DVP_IIR_t)->bounds[0] = bound;
            dvp_knode_to(&m_pNodes[15], DVP_IIR_t)->bounds[1] = bound;
            dvp_knode_to(&m_pNodes[15], DVP_IIR_t)->scratch = m_images[16];
            m_pNodes[15].header.affinity = DVP_CORE_DSP;

            m_pNodes[16].header.kernel = DVP_KN_VLIB_SUB_BACK_S16;
            dvp_knode_to(&m_pNodes[16], DVP_SubBackground_t)->newestData = m_images[1];
            dvp_knode_to(&m_pNodes[16], DVP_SubBackground_t)->output32packed = m_images[17];
            dvp_knode_to(&m_pNodes[16], DVP_SubBackground_t)->currentMean = m_images[10];
            dvp_knode_to(&m_pNodes[16], DVP_SubBackground_t)->currentVar = m_images[12];
            dvp_knode_to(&m_pNodes[16], DVP_SubBackground_t)->thresholdGlobal = CAMERA_NOISE_S16/32;
            dvp_knode_to(&m_pNodes[16], DVP_SubBackground_t)->thresholdFactor = THRESHOLD_FACTOR_S16/2;
            m_pNodes[16].header.affinity = DVP_CORE_DSP;

            m_pNodes[17].header.kernel = DVP_KN_VLIB_SUB_BACK_S32;
            dvp_knode_to(&m_pNodes[17], DVP_SubBackground_t)->newestData = m_images[1];
            dvp_knode_to(&m_pNodes[17], DVP_SubBackground_t)->output32packed = m_images[18];
            dvp_knode_to(&m_pNodes[17], DVP_SubBackground_t)->currentMean = m_images[11];
            dvp_knode_to(&m_pNodes[17], DVP_SubBackground_t)->currentVar = m_images[13];
            dvp_knode_to(&m_pNodes[17], DVP_SubBackground_t)->thresholdGlobal = CAMERA_NOISE_S32/32;
            dvp_knode_to(&m_pNodes[17], DVP_SubBackground_t)->thresholdFactor = THRESHOLD_FACTOR_S32/2;
            m_pNodes[17].header.affinity = DVP_CORE_DSP;

            m_pNodes[18].header.kernel = DVP_KN_VLIB_MOG_S16;
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->image = m_images[1];
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->wts = m_images[19];
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->means = m_images[20];
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->vars = m_images[21];
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->scratch = m_images[22];
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->compIndex = m_images[23];
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->fgmask = m_images[24];
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->alpha = updateRate1;
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->rho = updateRate2;
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->delta = mdThreshold;
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->T = bsThreshold;
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->ini_wt = initialWt;
            dvp_knode_to(&m_pNodes[18], DVP_Mog_t)->ini_var = initialVar;
            m_pNodes[18].header.affinity = DVP_CORE_DSP;

            m_pNodes[19].header.kernel = DVP_KN_VLIB_MOG_S32;
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->image = m_images[1];
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->wts = m_images[25];
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->means = m_images[26];
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->vars = m_images[27];
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->scratch = m_images[22];
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->compIndex = m_images[28];
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->fgmask = m_images[29];
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->alpha = updateRate1;
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->rho = updateRate2;
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->delta = mdThreshold;
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->T = bsThreshold;
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->ini_wt = initialWt;
            dvp_knode_to(&m_pNodes[19], DVP_Mog_t)->ini_var = initialVar;
            m_pNodes[19].header.affinity = DVP_CORE_DSP;

            m_pNodes[20].header.kernel = DVP_KN_VLIB_NORMALFLOW_16;
            dvp_knode_to(&m_pNodes[20], DVP_NormalFlow_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[20], DVP_NormalFlow_t)->mag = m_images[3];
            dvp_knode_to(&m_pNodes[20], DVP_NormalFlow_t)->x = m_images[3];
            dvp_knode_to(&m_pNodes[20], DVP_NormalFlow_t)->y = m_images[3];
            dvp_knode_to(&m_pNodes[20], DVP_NormalFlow_t)->lut = m_images[30];
            dvp_knode_to(&m_pNodes[20], DVP_NormalFlow_t)->u = m_images[31];
            dvp_knode_to(&m_pNodes[20], DVP_NormalFlow_t)->v = m_images[32];
            dvp_knode_to(&m_pNodes[20], DVP_NormalFlow_t)->thresh = 10;
            m_pNodes[20].header.affinity = DVP_CORE_DSP;

            m_pNodes[21].header.kernel = DVP_KN_VLIB_ERODE_SINGLEPIXEL;
            dvp_knode_to(&m_pNodes[21], DVP_Morphology_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[21], DVP_Morphology_t)->output = m_images[33];
            m_pNodes[21].header.affinity = DVP_CORE_DSP;

            m_pNodes[22].header.kernel = DVP_KN_UYVY_TO_YUV422p;
            dvp_knode_to(&m_pNodes[22], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[22], DVP_Transform_t)->output = m_images[34];
            m_pNodes[22].header.affinity = DVP_CORE_DSP;

            m_pNodes[23].header.kernel = DVP_KN_UYVY_TO_YUV420p;
            dvp_knode_to(&m_pNodes[23], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[23], DVP_Transform_t)->output = m_images[35];
            m_pNodes[23].header.affinity = DVP_CORE_DSP;

            m_pNodes[24].header.kernel = DVP_KN_YUV422p_TO_UYVY;
            dvp_knode_to(&m_pNodes[24], DVP_Transform_t)->input = m_images[34];
            dvp_knode_to(&m_pNodes[24], DVP_Transform_t)->output = m_images[36];
            m_pNodes[24].header.affinity = DVP_CORE_DSP;

            m_pNodes[25].header.kernel = DVP_KN_VLIB_UYVY_TO_LABp;
            dvp_knode_to(&m_pNodes[25], DVP_Int2Pl_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[25], DVP_Int2Pl_t)->output1 = m_images[37];
            dvp_knode_to(&m_pNodes[25], DVP_Int2Pl_t)->output2 = m_images[38];
            dvp_knode_to(&m_pNodes[25], DVP_Int2Pl_t)->output3 = m_images[39];
            dvp_knode_to(&m_pNodes[25], DVP_Int2Pl_t)->scratch = m_buffers[66];
            dvp_knode_to(&m_pNodes[25], DVP_Int2Pl_t)->factor = 4;
            m_pNodes[25].header.affinity = DVP_CORE_DSP;

            m_pNodes[26].header.kernel = DVP_KN_CANNY_2D_GRADIENT;
            dvp_knode_to(&m_pNodes[26], DVP_Canny2dGradient_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[26], DVP_Canny2dGradient_t)->outGradX = m_images[40];
            dvp_knode_to(&m_pNodes[26], DVP_Canny2dGradient_t)->outGradY = m_images[41];
            dvp_knode_to(&m_pNodes[26], DVP_Canny2dGradient_t)->outMag = m_images[42];
            m_pNodes[26].header.affinity = DVP_CORE_DSP;

            m_pNodes[27].header.kernel = DVP_KN_VLIB_HARRIS_SCORE_7x7;
            dvp_knode_to(&m_pNodes[27], DVP_HarrisDSP_t)->inGradX = m_images[40];
            dvp_knode_to(&m_pNodes[27], DVP_HarrisDSP_t)->inGradY = m_images[41];
            dvp_knode_to(&m_pNodes[27], DVP_HarrisDSP_t)->scratch = m_buffers[0];
            dvp_knode_to(&m_pNodes[27], DVP_HarrisDSP_t)->harrisScore = m_images[43];
            dvp_knode_to(&m_pNodes[27], DVP_HarrisDSP_t)->k = 1310;
            m_pNodes[27].header.affinity = DVP_CORE_DSP;

            m_pNodes[28].header.kernel = DVP_KN_VLIB_TRACK_FEATURES_LUCAS_7x7;
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->im1 = m_images[1];
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->im2 = m_images[1];
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->gradx = m_images[40];
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->grady = m_images[41];
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->x = m_buffers[8];
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->y = m_buffers[9];
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->outx = m_buffers[10];
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->outy = m_buffers[11];
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->scratch = m_buffers[12];
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->nfeatures = NFEATURES;
            dvp_knode_to(&m_pNodes[28], DVP_TrackFeatures_t)->max_iters = 10;

            m_pNodes[29].header.kernel = DVP_KN_VLIB_HOUGH_LINE_FROM_LIST;
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->pEdgeMapList = m_buffers[1];
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->pOutHoughSpace = m_images[44];
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->pSIN = m_buffers[2];
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->pCOS = m_buffers[3];
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->ping = m_buffers[4];
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->pong = m_buffers[5];
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->pang = m_buffers[6];
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->peng = m_buffers[7];
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->outBlkWidth = m_width;
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->outBlkHeight = m_height;
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->listSize = m_height*m_width*.05;
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->thetaRange = 267;
            dvp_knode_to(&m_pNodes[29], DVP_HoughLine_t)->rhoMaxLength = 267;
            m_pNodes[29].header.affinity = DVP_CORE_DSP;

            m_pNodes[30].header.kernel = DVP_KN_VLIB_EWR_MEAN_S16;
            dvp_knode_to(&m_pNodes[30], DVP_EMean_t)->mean = m_images[10];
            dvp_knode_to(&m_pNodes[30], DVP_EMean_t)->data = m_images[1];
            dvp_knode_to(&m_pNodes[30], DVP_EMean_t)->mask = m_images[45];
            dvp_knode_to(&m_pNodes[30], DVP_EMean_t)->weight = IIR_ALPHA_S16;
            m_pNodes[30].header.affinity = DVP_CORE_DSP;

            m_pNodes[31].header.kernel = DVP_KN_VLIB_EWR_MEAN_S32;
            dvp_knode_to(&m_pNodes[31], DVP_EMean_t)->mean = m_images[11];
            dvp_knode_to(&m_pNodes[31], DVP_EMean_t)->data = m_images[1];
            dvp_knode_to(&m_pNodes[31], DVP_EMean_t)->mask = m_images[46];
            dvp_knode_to(&m_pNodes[31], DVP_EMean_t)->weight = IIR_ALPHA_S32;
            m_pNodes[31].header.affinity = DVP_CORE_DSP;

            m_pNodes[32].header.kernel = DVP_KN_VLIB_EWR_VAR_S16;
            dvp_knode_to(&m_pNodes[32], DVP_EMean_t)->var = m_images[12];
            dvp_knode_to(&m_pNodes[32], DVP_EMean_t)->mean = m_images[10];
            dvp_knode_to(&m_pNodes[32], DVP_EMean_t)->data = m_images[1];
            dvp_knode_to(&m_pNodes[32], DVP_EMean_t)->mask = m_images[47];
            dvp_knode_to(&m_pNodes[32], DVP_EMean_t)->weight = IIR_ALPHA_S16;
            m_pNodes[32].header.affinity = DVP_CORE_DSP;

            m_pNodes[33].header.kernel = DVP_KN_VLIB_EWR_VAR_S32;
            dvp_knode_to(&m_pNodes[33], DVP_EMean_t)->var = m_images[13];
            dvp_knode_to(&m_pNodes[33], DVP_EMean_t)->mean = m_images[11];
            dvp_knode_to(&m_pNodes[33], DVP_EMean_t)->data = m_images[1];
            dvp_knode_to(&m_pNodes[33], DVP_EMean_t)->mask = m_images[48];
            dvp_knode_to(&m_pNodes[33], DVP_EMean_t)->weight = IIR_ALPHA_S32;
            m_pNodes[33].header.affinity = DVP_CORE_DSP;

            m_pNodes[34].header.kernel = DVP_KN_VLIB_UWR_MEAN_S16;
            dvp_knode_to(&m_pNodes[34], DVP_UMean_t)->old_mean = m_images[10];
            dvp_knode_to(&m_pNodes[34], DVP_UMean_t)->new_mean = m_images[10];
            dvp_knode_to(&m_pNodes[34], DVP_UMean_t)->old_data = m_images[1];
            dvp_knode_to(&m_pNodes[34], DVP_UMean_t)->new_data = m_images[1];
            dvp_knode_to(&m_pNodes[34], DVP_UMean_t)->old_mask = m_images[49];
            dvp_knode_to(&m_pNodes[34], DVP_UMean_t)->new_mask = m_images[49];
            dvp_knode_to(&m_pNodes[34], DVP_UMean_t)->bufferLength = 10;
            m_pNodes[34].header.affinity = DVP_CORE_DSP;

            m_pNodes[35].header.kernel = DVP_KN_VLIB_UWR_VAR_S16;
            dvp_knode_to(&m_pNodes[35], DVP_UMean_t)->old_var = m_images[12];
            dvp_knode_to(&m_pNodes[35], DVP_UMean_t)->new_var = m_images[12];
            dvp_knode_to(&m_pNodes[35], DVP_UMean_t)->old_mean = m_images[10];
            dvp_knode_to(&m_pNodes[35], DVP_UMean_t)->new_mean = m_images[10];
            dvp_knode_to(&m_pNodes[35], DVP_UMean_t)->old_data = m_images[1];
            dvp_knode_to(&m_pNodes[35], DVP_UMean_t)->new_data = m_images[1];
            dvp_knode_to(&m_pNodes[35], DVP_UMean_t)->old_mask = m_images[50];
            dvp_knode_to(&m_pNodes[35], DVP_UMean_t)->new_mask = m_images[50];
            dvp_knode_to(&m_pNodes[35], DVP_UMean_t)->bufferLength = 10;
            m_pNodes[35].header.affinity = DVP_CORE_DSP;

            m_pNodes[36].header.kernel = DVP_KN_VLIB_LEGENDRE_MOMENTS_INIT;
            dvp_knode_to(&m_pNodes[36], DVP_LegendreMoments_t)->lPoly = m_buffers[13];
            dvp_knode_to(&m_pNodes[36], DVP_LegendreMoments_t)->order = LGM_ORDER;

            m_pNodes[37].header.kernel = DVP_KN_VLIB_LEGENDRE_MOMENTS;
            dvp_knode_to(&m_pNodes[37], DVP_LegendreMoments_t)->im = m_images[2];
            dvp_knode_to(&m_pNodes[37], DVP_LegendreMoments_t)->lPoly = m_buffers[13];
            dvp_knode_to(&m_pNodes[37], DVP_LegendreMoments_t)->lMoments = m_buffers[14];
            dvp_knode_to(&m_pNodes[37], DVP_LegendreMoments_t)->order = LGM_ORDER;

            m_pNodes[38].header.kernel = DVP_KN_NV12_TO_YUV444p;
            dvp_knode_to(&m_pNodes[38], DVP_Transform_t)->input = m_images[53];
            dvp_knode_to(&m_pNodes[38], DVP_Transform_t)->output = m_images[54];
            m_pNodes[38].header.affinity = DVP_CORE_DSP;

            m_pNodes[39].header.kernel = DVP_KN_CANNY_NONMAX_SUPPRESSION;
            dvp_knode_to(&m_pNodes[39], DVP_CannyNonMaxSuppression_t)->inGradX = m_images[40];
            dvp_knode_to(&m_pNodes[39], DVP_CannyNonMaxSuppression_t)->inGradY = m_images[41];
            dvp_knode_to(&m_pNodes[39], DVP_CannyNonMaxSuppression_t)->inMag = m_images[42];
            dvp_knode_to(&m_pNodes[39], DVP_CannyNonMaxSuppression_t)->output = m_images[55];
            m_pNodes[39].header.affinity = DVP_CORE_DSP;

            m_pNodes[40].header.kernel = DVP_KN_VLIB_HYST_THRESHOLD;
            dvp_knode_to(&m_pNodes[40], DVP_CannyHystThresholding_t)->inMag = m_images[42];
            dvp_knode_to(&m_pNodes[40], DVP_CannyHystThresholding_t)->inEdgeMap = m_images[55];
            dvp_knode_to(&m_pNodes[40], DVP_CannyHystThresholding_t)->output = m_images[56]; //scratch
            dvp_knode_to(&m_pNodes[40], DVP_CannyHystThresholding_t)->loThresh = 35;
            dvp_knode_to(&m_pNodes[40], DVP_CannyHystThresholding_t)->hiThresh = 122;

            m_pNodes[41].header.kernel = DVP_KN_VLIB_KALMAN_2x4;
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->inMeasurements = m_buffers[15];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->transition = m_buffers[16];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->errorCov = m_buffers[17];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->predictedErrorCov = m_buffers[18];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->state = m_buffers[19];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->predictedState = m_buffers[20];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->measurement = m_buffers[21];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->processNoiseCov = m_buffers[22];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->measurementNoiseCov = m_buffers[23];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->kalmanGain = m_buffers[24];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->temp1 = m_buffers[25];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->temp2 = m_buffers[26];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->temp3 = m_buffers[27];
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->nMeasurements = m_width*m_height;
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->sD = sD_2x4;
            dvp_knode_to(&m_pNodes[41], DVP_KalmanFilter_t)->mD = mD_2x4;

            m_pNodes[42].header.kernel = DVP_KN_VLIB_KALMAN_4x6;
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->inMeasurements = m_buffers[28];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->transition = m_buffers[29];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->errorCov = m_buffers[30];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->predictedErrorCov = m_buffers[31];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->state = m_buffers[32];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->predictedState = m_buffers[33];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->measurement = m_buffers[34];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->processNoiseCov = m_buffers[35];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->measurementNoiseCov = m_buffers[36];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->kalmanGain = m_buffers[37];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->temp1 = m_buffers[38];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->temp2 = m_buffers[39];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->temp3 = m_buffers[40];
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->scaleFactor = 5;
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->nMeasurements = m_width*m_height;
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->sD = sD_4x6;
            dvp_knode_to(&m_pNodes[42], DVP_KalmanFilter_t)->mD = mD_4x6;

            m_pNodes[43].header.kernel = DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_16;
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->start = m_buffers[41];
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->step = m_buffers[42];
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->stop = m_buffers[43];
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->intBuf[0] = m_buffers[44];
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->intBuf[1] = m_buffers[45];
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->intBuf[2] = m_buffers[46];
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->intBuf[3] = m_buffers[47];
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->intBuf[4] = m_buffers[48];
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->intBuf[5] = m_buffers[49];
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->N_val = SimplexN;
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->N_INV = 32767 / SimplexN;
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->MaxIteration = 200;
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->EPSILON = (DVP_S32)(0.00001 * ((DVP_U32)(MAX_S32_S+1)/MAX_4p27));
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->addtlArgs.n = SimplexN;
            dvp_knode_to(&m_pNodes[43], DVP_Simplex_t)->addtlArgs.coeff = 10;

            m_pNodes[44].header.kernel = DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_3D;
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->start = m_buffers[50];
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->step = m_buffers[51];
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->stop = m_buffers[52];
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->intBuf[0] = m_buffers[53];
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->intBuf[1] = m_buffers[54];
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->intBuf[2] = m_buffers[55];
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->intBuf[3] = m_buffers[56];
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->intBuf[4] = m_buffers[57];
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->intBuf[5] = m_buffers[58];
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->MaxIteration = 200;
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->EPSILON = (DVP_S32)(0.00001 * ((DVP_U32)(MAX_S32_S+1)/MAX_4p27));
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->addtlArgs.n = SimplexN;
            dvp_knode_to(&m_pNodes[44], DVP_Simplex_t)->addtlArgs.coeff = 10;

            m_pNodes[45].header.kernel = DVP_KN_VLIB_INTEGRAL_IMAGE_16;
            dvp_knode_to(&m_pNodes[45], DVP_Transform_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[45], DVP_Transform_t)->output = m_images[57];

            m_pNodes[46].header.kernel = DVP_KN_VLIB_EXTRACT_BACK_8_16;
            dvp_knode_to(&m_pNodes[46], DVP_Transform_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[46], DVP_Transform_t)->output = m_images[58];

            if((m_testcore == DVP_CORE_DSP) || (m_testcore == DVP_CORE_CPU))
            {
                m_pNodes[47].header.kernel = DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_8;
                dvp_knode_to(&m_pNodes[47], DVP_Gradient_t)->input = m_images[1];
                dvp_knode_to(&m_pNodes[47], DVP_Gradient_t)->output = m_images[59];
                dvp_knode_to(&m_pNodes[47], DVP_Gradient_t)->scratch = m_images[61];
            }
#if defined(DVP_USE_VRUN)
            else
            {
                m_pNodes[47].header.kernel = DVP_KN_VRUN_GAUSSIAN_5x5_PYRAMID_8;
                dvp_knode_to(&m_pNodes[47], DVP_Transform_t)->input = m_images[1];
                dvp_knode_to(&m_pNodes[47], DVP_Transform_t)->output = m_images[59];
            }
#endif
            m_pNodes[48].header.kernel = DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_16;
            dvp_knode_to(&m_pNodes[48], DVP_Gradient_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[48], DVP_Gradient_t)->output = m_images[60];
            dvp_knode_to(&m_pNodes[48], DVP_Gradient_t)->scratch = m_images[61];

            m_pNodes[49].header.kernel = DVP_KN_VLIB_HISTOGRAM_ND_16;
            dvp_knode_to(&m_pNodes[49], DVP_Histogram_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[49], DVP_Histogram_t)->numBinsArray = m_buffers[59];
            dvp_knode_to(&m_pNodes[49], DVP_Histogram_t)->h[0] = m_buffers[60];
            dvp_knode_to(&m_pNodes[49], DVP_Histogram_t)->h[1] = m_buffers[61];
            dvp_knode_to(&m_pNodes[49], DVP_Histogram_t)->h[2] = m_buffers[62];
            dvp_knode_to(&m_pNodes[49], DVP_Histogram_t)->hOut = m_buffers[63];
            dvp_knode_to(&m_pNodes[49], DVP_Histogram_t)->dimX = HIST_ND_DIMX;
            dvp_knode_to(&m_pNodes[49], DVP_Histogram_t)->binWeight = 1;
            dvp_knode_to(&m_pNodes[49], DVP_Histogram_t)->clearFlag = 0;
            m_buffers[59].pData[0] = 32; m_buffers[59].pData[1] = 32; m_buffers[59].pData[2] = 32; m_buffers[59].pData[3] = 32;
            tmpPtr = (DVP_S16*)m_buffers[60].pData;
            tmpPtr[0] = 256; tmpPtr[1] = 256; tmpPtr[2] = 256; tmpPtr[3] = 256;

            m_pNodes[50].header.kernel = DVP_KN_VLIB_PACK_MASK_32;
            dvp_knode_to(&m_pNodes[50], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[50], DVP_Transform_t)->output = m_images[62];

            m_pNodes[51].header.kernel = DVP_KN_VLIB_UNPACK_MASK_32;
            dvp_knode_to(&m_pNodes[51], DVP_Transform_t)->input = m_images[62];
            dvp_knode_to(&m_pNodes[51], DVP_Transform_t)->output = m_images[63];

            m_pNodes[52].header.kernel = DVP_KN_VLIB_HARRIS_SCORE_7x7_U32;
            dvp_knode_to(&m_pNodes[52], DVP_HarrisDSP_t)->inGradX = m_images[40];
            dvp_knode_to(&m_pNodes[52], DVP_HarrisDSP_t)->inGradY = m_images[41];
            dvp_knode_to(&m_pNodes[52], DVP_HarrisDSP_t)->scratch = m_buffers[0];
            dvp_knode_to(&m_pNodes[52], DVP_HarrisDSP_t)->harrisScore = m_images[64];
            dvp_knode_to(&m_pNodes[52], DVP_HarrisDSP_t)->k = 1310;

            m_pNodes[53].header.kernel = DVP_KN_VLIB_NONMAXSUPPRESS_U32;
            dvp_knode_to(&m_pNodes[53], DVP_Nonmax_NxN_t)->input = m_images[64];
            dvp_knode_to(&m_pNodes[53], DVP_Nonmax_NxN_t)->pixIndex = m_buffers[64];
            dvp_knode_to(&m_pNodes[53], DVP_Nonmax_NxN_t)->scratch = m_buffers[65];
            dvp_knode_to(&m_pNodes[53], DVP_Nonmax_NxN_t)->threshold = 100;
            dvp_knode_to(&m_pNodes[53], DVP_Nonmax_NxN_t)->p = 5;

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;

        }

        if(m_testcore != DVP_CORE_MIN) {
            for(uint32_t testnode=0; testnode<m_numNodes; testnode++) {
                m_pNodes[testnode].header.affinity = m_testcore;
            }
        }

        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
           if (m_imgdbg_enabled && AllocateImageDebug(67))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_luma16bitX");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "02_luma16bitY");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "03_HSL_H");
                ImageDebug_Init(&m_imgdbg[5], &m_images[6], m_imgdbg_path, "04_GradPyrV5");
                ImageDebug_Init(&m_imgdbg[6], &m_images[7], m_imgdbg_path, "05_GradPyrH5");
                ImageDebug_Init(&m_imgdbg[7], &m_images[8], m_imgdbg_path, "06_Pyramid8");
                ImageDebug_Init(&m_imgdbg[8], &m_images[9], m_imgdbg_path, "07_Pyramid16");
                ImageDebug_Init(&m_imgdbg[9], &m_images[10], m_imgdbg_path, "08_initMean16");
                ImageDebug_Init(&m_imgdbg[10], &m_images[11], m_imgdbg_path, "09_initMean32");
                ImageDebug_Init(&m_imgdbg[11], &m_images[12], m_imgdbg_path, "10_initVar16");
                ImageDebug_Init(&m_imgdbg[12], &m_images[13], m_imgdbg_path, "11_initVar32");
                ImageDebug_Init(&m_imgdbg[13], &m_images[14], m_imgdbg_path, "12_IIRH16");
                ImageDebug_Init(&m_imgdbg[14], &m_images[15], m_imgdbg_path, "13_IIRV16");
                ImageDebug_Init(&m_imgdbg[15], &m_images[17], m_imgdbg_path, "14_subBack16");
                ImageDebug_Init(&m_imgdbg[16], &m_images[18], m_imgdbg_path, "15_subBack32");
                ImageDebug_Init(&m_imgdbg[17], &m_images[19], m_imgdbg_path, "16_mog16Wts");
                ImageDebug_Init(&m_imgdbg[18], &m_images[20], m_imgdbg_path, "16_mog16Means");
                ImageDebug_Init(&m_imgdbg[19], &m_images[21], m_imgdbg_path, "16_mog16Vars");
                ImageDebug_Init(&m_imgdbg[20], &m_images[23], m_imgdbg_path, "16_mog16CompIdX");
                ImageDebug_Init(&m_imgdbg[21], &m_images[24], m_imgdbg_path, "16_mog16Fgmask");
                ImageDebug_Init(&m_imgdbg[22], &m_images[25], m_imgdbg_path, "17_mog32Wts");
                ImageDebug_Init(&m_imgdbg[23], &m_images[26], m_imgdbg_path, "17_mog32Means");
                ImageDebug_Init(&m_imgdbg[24], &m_images[27], m_imgdbg_path, "17_mog32Vars");
                ImageDebug_Init(&m_imgdbg[25], &m_images[28], m_imgdbg_path, "17_mog32CompIdX");
                ImageDebug_Init(&m_imgdbg[26], &m_images[29], m_imgdbg_path, "17_mog32Fgmask");
                ImageDebug_Init(&m_imgdbg[27], &m_images[31], m_imgdbg_path, "18_normFlow_u");
                ImageDebug_Init(&m_imgdbg[28], &m_images[32], m_imgdbg_path, "18_normFlow_v");
                ImageDebug_Init(&m_imgdbg[29], &m_images[33], m_imgdbg_path, "19_erodeSinglePiXel");
                ImageDebug_Init(&m_imgdbg[30], &m_images[34], m_imgdbg_path, "20_422pl");
                ImageDebug_Init(&m_imgdbg[31], &m_images[35], m_imgdbg_path, "21_420pl");
                ImageDebug_Init(&m_imgdbg[32], &m_images[36], m_imgdbg_path, "22_420plUYVY");
                ImageDebug_Init(&m_imgdbg[33], &m_images[37], m_imgdbg_path, "23_LABpL");
                ImageDebug_Init(&m_imgdbg[34], &m_images[38], m_imgdbg_path, "23_LABpa");
                ImageDebug_Init(&m_imgdbg[35], &m_images[39], m_imgdbg_path, "23_LABpb");
                ImageDebug_Init(&m_imgdbg[36], &m_images[40], m_imgdbg_path, "24_XyGradX");
                ImageDebug_Init(&m_imgdbg[37], &m_images[41], m_imgdbg_path, "24_XyGradY");
                ImageDebug_Init(&m_imgdbg[38], &m_images[42], m_imgdbg_path, "24_XyMag");
                ImageDebug_Init(&m_imgdbg[39], &m_images[43], m_imgdbg_path, "25_harrisScore");
                ImageDebug_Init(&m_imgdbg[40], &m_images[44], m_imgdbg_path, "26_houghSpace");
                ImageDebug_Init(&m_imgdbg[41], &m_images[45], m_imgdbg_path, "27_updateEWRMean16");
                ImageDebug_Init(&m_imgdbg[42], &m_images[46], m_imgdbg_path, "28_updateEWRMean32");
                ImageDebug_Init(&m_imgdbg[43], &m_images[47], m_imgdbg_path, "29_updateEWRVar16");
                ImageDebug_Init(&m_imgdbg[44], &m_images[48], m_imgdbg_path, "30_updateEWRVar32");
                ImageDebug_Init(&m_imgdbg[45], &m_images[51], m_imgdbg_path, "03_HSL_S");
                ImageDebug_Init(&m_imgdbg[46], &m_images[52], m_imgdbg_path, "03_HSL_L");
                ImageDebug_Init(&m_imgdbg[47], &m_images[53], m_imgdbg_path, "31_NV12");
                ImageDebug_Init(&m_imgdbg[48], &m_images[54], m_imgdbg_path, "31_NV12to422PLout");
                ImageDebug_Init(&m_imgdbg[49], &m_images[55], m_imgdbg_path, "32_cannyNON");
                ImageDebug_Init(&m_imgdbg[58], &m_images[57], m_imgdbg_path, "40_integral16");
                ImageDebug_Init(&m_imgdbg[59], &m_images[58], m_imgdbg_path, "41_extractBack16");
                ImageDebug_Init(&m_imgdbg[60], &m_images[59], m_imgdbg_path, "42_gaus5X5pyramid8");
                ImageDebug_Init(&m_imgdbg[61], &m_images[60], m_imgdbg_path, "43_gaus5X5pyramid16");
                ImageDebug_Init(&m_imgdbg[63], &m_images[62], m_imgdbg_path, "44_pack");
                ImageDebug_Init(&m_imgdbg[64], &m_images[63], m_imgdbg_path, "45_unpack");
                ImageDebug_Init(&m_imgdbg[65], &m_images[64], m_imgdbg_path, "46_harris32");
                BufferDebug_Init(&m_imgdbg[50], &m_buffers[13], m_imgdbg_path, "34_legendrePoly", ".bw");
                BufferDebug_Init(&m_imgdbg[51], &m_buffers[14], m_imgdbg_path, "34_legendreMom", ".bw");
                BufferDebug_Init(&m_imgdbg[52], &m_buffers[10], m_imgdbg_path, "35_lucaskanadeX", ".bw");
                BufferDebug_Init(&m_imgdbg[53], &m_buffers[11], m_imgdbg_path, "35_lucaskanadeY", ".bw");
                BufferDebug_Init(&m_imgdbg[54], &m_buffers[19], m_imgdbg_path, "36_kalman2X4", ".bw");
                BufferDebug_Init(&m_imgdbg[55], &m_buffers[32], m_imgdbg_path, "37_kalman4X6", ".bw");
                BufferDebug_Init(&m_imgdbg[56], &m_buffers[43], m_imgdbg_path, "38_simplex16", ".bw");
                BufferDebug_Init(&m_imgdbg[57], &m_buffers[52], m_imgdbg_path, "39_simplex3D", ".bw");
                BufferDebug_Init(&m_imgdbg[62], &m_buffers[63], m_imgdbg_path, "44_Hist_nD16", ".bw");
                BufferDebug_Init(&m_imgdbg[66], &m_buffers[64], m_imgdbg_path, "47_nms32", ".bw");
                BufferDebug_Init(&m_imgdbg[66], &m_buffers[66], m_imgdbg_path, "48_LabLUT", ".bw");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
    status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;

}
status_e TestVisionEngine::Test_NV12GraphSetup()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_YUV)
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e dispType = DVP_MTYPE_DEFAULT;

#if defined(DVP_USE_TILER) || defined(DVP_USE_BO)
    opType = DVP_MTYPE_MPUCACHED_1DTILED;
#elif defined(DVP_USE_ION)
    opType = DVP_MTYPE_MPUNONCACHED_2DTILED;
#endif

    // set the code we want the camera and display to use
    m_fourcc = FOURCC_NV12;

    DVP_PRINT(DVP_ZONE_ENGINE, "NV12 Graph! (optype=%d)\n",opType);

    if (m_hDVP)
    {
        if (AllocateImageStructs(11))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, m_fourcc);
            DVP_Image_Init(&m_images[1], m_width, m_height, m_fourcc);
            DVP_Image_Init(&m_images[2], m_width, m_height, m_fourcc);
            DVP_Image_Init(&m_images[3], m_width, m_height, m_fourcc);
            DVP_Image_Init(&m_images[4], m_width/2, m_height/2, FOURCC_YU24);   // Downsize Luma
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_YU24);       // Upsize Chroma
            DVP_Image_Init(&m_images[6], m_width, m_height, FOURCC_UYVY); //UYVY
            DVP_Image_Init(&m_images[7], m_width, m_height, FOURCC_Y16); //H
            DVP_Image_Init(&m_images[8], m_width, m_height, FOURCC_Y800); //S
            DVP_Image_Init(&m_images[9], m_width, m_height, FOURCC_Y800); //V
            DVP_Image_Init(&m_images[10], m_width, m_height, FOURCC_UYVY); //UYVY
            m_camIdx = m_dispIdx = 0;
            m_camMin = m_dispMin = 0;
            m_camMax = m_dispMax = 4;
            if (m_display_enabled)
            {
                DisplayCreate(m_width, m_height, m_width, m_height, m_width, m_height, 0, 0);
                for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++)
                    DisplayAllocate(&m_images[m_dispIdx]);
                dispType = (DVP_MemType_e)m_images[m_dispMin].memType;
            }
#if defined(DVP_USE_GRALLOC)
            else {
                dispType = DVP_MTYPE_GRALLOC_2DTILED;
            }
#elif defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
            else {
                dispType = DVP_MTYPE_MPUNONCACHED_2DTILED;
            }
#endif
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0],dispType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1],dispType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2],dispType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3],dispType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[8], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[9], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[10], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(4) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_NV12_TO_YUV444p;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[4];

            m_pNodes[1].header.kernel = DVP_KN_NV12_TO_YUV444p;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[5];

            m_pNodes[2].header.kernel = DVP_KN_YUV444p_TO_UYVY;
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->input = m_images[5];//444p
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->output = m_images[6];//UYVY

            m_pNodes[3].header.kernel = DVP_KN_UYVY_TO_HSLp;
            dvp_knode_to(&m_pNodes[3], DVP_Int2Pl_t)->input = m_images[6];
            dvp_knode_to(&m_pNodes[3], DVP_Int2Pl_t)->output1 = m_images[7];
            dvp_knode_to(&m_pNodes[3], DVP_Int2Pl_t)->output2 = m_images[8];
            dvp_knode_to(&m_pNodes[3], DVP_Int2Pl_t)->output3 = m_images[9];
            //m_pNodes[3].affinity = DVP_CORE_DSP;

            //DVP_KN_NV12_TO_UYVY
            //m_pNodes[4].header.kernel = DVP_KN_NV12_TO_UYVY;
            //dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->input = m_images[m_camIdx];//NV12
            //dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->output = m_images[10];//UYVY

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_fourcc);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(8))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[4], m_imgdbg_path, "01_YUV444");
                ImageDebug_Init(&m_imgdbg[2], &m_images[5], m_imgdbg_path, "01_YUV444");
                ImageDebug_Init(&m_imgdbg[3], &m_images[6], m_imgdbg_path, "02_UYVY");
                ImageDebug_Init(&m_imgdbg[4], &m_images[7], m_imgdbg_path, "03_H");
                ImageDebug_Init(&m_imgdbg[5], &m_images[8], m_imgdbg_path, "03_S");
                ImageDebug_Init(&m_imgdbg[6], &m_images[9], m_imgdbg_path, "03_V");
                ImageDebug_Init(&m_imgdbg[7], &m_images[10], m_imgdbg_path, "04_UYVY");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_UYVYScalingGraph()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_YUV)
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;

#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    opType = DVP_MTYPE_MPUCACHED_1DTILED;
#endif
    if (m_display_enabled)
        m_display = DVP_Display_Create(m_width, m_height, m_width, m_height, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, m_width, m_height, 0, 0, FOURCC_UYVY, 0, DVP_DISPLAY_NUM_BUFFERS);
    if (m_hDVP)
    {
        if (AllocateImageStructs(6))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[4], m_width/2, m_height/2, FOURCC_UYVY);
            DVP_Image_Init(&m_images[5], m_width/4, m_height/4, FOURCC_UYVY);
            m_camIdx = m_dispIdx = 0;
            m_camMin = m_dispMin = 0;
            m_camMax = m_dispMax = 4;
            if (m_display_enabled)
            {
                DVP_Display_Alloc(m_display, &m_images[0]);
                DVP_Display_Alloc(m_display, &m_images[1]);
                DVP_Display_Alloc(m_display, &m_images[2]);
                DVP_Display_Alloc(m_display, &m_images[3]);
            }
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
            else {
                m_images[0].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
                m_images[1].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
                m_images[2].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
                m_images[3].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
            }
#endif
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0],(DVP_MemType_e)m_images[0].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1],(DVP_MemType_e)m_images[1].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2],(DVP_MemType_e)m_images[2].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3],(DVP_MemType_e)m_images[3].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(2) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_YUV_UYVY_HALF_SCALE;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[4];

            m_pNodes[1].header.kernel = DVP_KN_YUV_UYVY_QTR_SCALE;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[5];

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(2))
            {
                //ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[0], &m_images[4], m_imgdbg_path, "01_half");
                ImageDebug_Init(&m_imgdbg[1], &m_images[5], m_imgdbg_path, "02_qtr");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}


status_e TestVisionEngine::Test_Rotate()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_YUV)
    if (m_display_enabled)
        m_display = DVP_Display_Create(m_width, m_height, m_width, m_height, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, m_width, m_height, 0, 0, FOURCC_UYVY, 0, DVP_DISPLAY_NUM_BUFFERS);
    if (m_hDVP)
    {
        if (AllocateImageStructs(9))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_IYUV);
            DVP_Image_Init(&m_images[5], m_height, m_width, FOURCC_IYUV);
            DVP_Image_Init(&m_images[6], m_height, m_width, FOURCC_IYUV);
            DVP_Image_Init(&m_images[7], m_height, m_width, FOURCC_UYVY);
            DVP_Image_Init(&m_images[8], m_height, m_width, FOURCC_UYVY);
            m_camIdx = m_dispIdx = 0;
            m_camMin = m_dispMin = 0;
            m_camMax = m_dispMax = 4;
            if (m_display_enabled)
            {
                DVP_Display_Alloc(m_display, &m_images[0]);
                DVP_Display_Alloc(m_display, &m_images[1]);
                DVP_Display_Alloc(m_display, &m_images[2]);
                DVP_Display_Alloc(m_display, &m_images[3]);
            }
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
            else {
                m_images[0].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
                m_images[1].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
                m_images[2].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
                m_images[3].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
            }
#endif
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0],(DVP_MemType_e)m_images[0].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1],(DVP_MemType_e)m_images[1].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2],(DVP_MemType_e)m_images[2].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3],(DVP_MemType_e)m_images[3].memType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], DVP_MTYPE_MPUCACHED_VIRTUAL) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], DVP_MTYPE_MPUCACHED_VIRTUAL) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], DVP_MTYPE_MPUCACHED_VIRTUAL) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[8], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(5) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_UYVY_TO_YUV420p;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[4];
            m_pNodes[0].header.affinity = DVP_CORE_CPU;

            m_pNodes[1].header.kernel = DVP_KN_YUV_Y800_ROTATE_CW_90;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[4];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[5];

            m_pNodes[2].header.kernel = DVP_KN_YUV_Y800_ROTATE_CCW_90;
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->input = m_images[4];
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->output = m_images[6];

            m_pNodes[3].header.kernel = DVP_KN_YUV_UYVY_ROTATE_CW_90;
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->output = m_images[7];

            m_pNodes[4].header.kernel = DVP_KN_YUV_UYVY_ROTATE_CCW_90;
            dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->output = m_images[8];

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;

        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(5))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[4], m_imgdbg_path, "01_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[5], m_imgdbg_path, "02_cw90");
                ImageDebug_Init(&m_imgdbg[2], &m_images[6], m_imgdbg_path, "03_ccw90");
                ImageDebug_Init(&m_imgdbg[3], &m_images[7], m_imgdbg_path, "04_cw90");
                ImageDebug_Init(&m_imgdbg[4], &m_images[8], m_imgdbg_path, "05_ccw90");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_Disparity()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VLIB) && defined(DVP_USE_YUV)
    if (m_hDVP)
    {
        if (m_pCam == NULL)
        {
            m_pCam = VisionCamFactory(m_camtype);
            if (m_pCam == NULL)
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        VisionCamStereoInfo info = {VCAM_STEREO_LAYOUT_LEFTRIGHT, 1};
        //VisionCamWhiteBalType white = VCAM_WHITE_BAL_CONTROL_AUTO;
        VisionCamResType res;
        uint32_t color = FOURCC_NV12;
        m_sensorIndex = VCAM_SENSOR_STEREO;
        m_capmode = VCAM_STEREO_MODE;
        //int brightness = 50;
        int32_t allocWidth;
        int32_t allocHeight;

#if 0   // Explicity specify combined width/height on test command line
        if (info.layout == VCAM_STEREO_LAYOUT_LEFTRIGHT && info.subsampling == 1) {
            m_width *= 2;
        } else if (info.layout == VCAM_STEREO_LAYOUT_TOPBOTTOM && info.subsampling == 1) {
            m_height *= 2;
        }
#endif

        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->init(this));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_WIDTH, &m_width, sizeof(uint32_t)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_HEIGHT, &m_height, sizeof(uint32_t)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_COLOR_SPACE_FOURCC, &color, sizeof(fourcc_t)));
        // Can't set ROTATION here, see below
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_FPS_FIXED, &m_fps, sizeof(uint32_t)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_NAME, m_name, (int32_t)strlen(m_name)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_CAP_MODE, &m_capmode, sizeof(m_capmode)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_SENSOR_SELECT, &m_sensorIndex, sizeof( m_sensorIndex) ));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_STEREO_INFO, &info, sizeof(info)));

        memset(&res, 0, sizeof(res));
        // VCAM_PARAM_2DBUFFER_DIM should only be called after resolutions, color space, cap mode, and
        //   optionally stereo information is set.
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->getParameter(VCAM_PARAM_2DBUFFER_DIM, &res, sizeof(res)));

        allocWidth = res.mWidth;
        allocHeight = res.mHeight;

        m_camIdx = m_dispIdx = 0;
        m_camMin = m_dispMin = 0;
        m_camMax = m_dispMax = 4;

        if (m_display_enabled)
            m_display = DVP_Display_Create(m_width, m_height, allocWidth, allocHeight, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, allocWidth, allocHeight, 0, 0, color, 0, m_dispMax+1);

        if (AllocateImageStructs(m_dispMax+4))
        {
            for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++)
            {
                DVP_Image_Init(&m_images[m_dispIdx], allocWidth, allocHeight, color);
                m_images[m_dispIdx].width = m_width;
                m_images[m_dispIdx].height = m_height;
            }

            DVP_Image_Init(&m_images[m_dispMax+0], m_width/2, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[m_dispMax+1], m_width/2, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[m_dispMax+2], m_width/2, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[m_dispMax+3], m_width/2, m_height/2, FOURCC_YU24);

            if (m_display_enabled) {
                for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++)
                    if (DVP_Display_Alloc(m_display, &m_images[m_dispIdx]) == DVP_FALSE)
                        return STATUS_NOT_ENOUGH_MEMORY;
            }
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
            else {
                for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++)
                    m_images[m_dispIdx].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
            }
#endif
            for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++) {
                if (DVP_Image_Alloc(m_hDVP, &m_images[m_dispIdx],(DVP_MemType_e)m_images[m_dispIdx].memType) == DVP_FALSE)
                    return STATUS_NOT_ENOUGH_MEMORY;
            }

            if (!DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+0], DVP_MTYPE_MPUCACHED_VIRTUAL) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+1], DVP_MTYPE_MPUCACHED_VIRTUAL) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+2], DVP_MTYPE_MPUCACHED_VIRTUAL) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+3], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

//        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_FLICKER,&m_flicker,sizeof(m_flicker)));
//        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_BRIGHTNESS, &brightness, sizeof(brightness)));
//        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_AWB_MODE, &white, sizeof(white)));
//        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_NAME, m_name, sizeof(m_name)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->useBuffers(&m_images[m_camIdx], m_camMax));
        // @todo BUG: Can't set ROTATION until after useBuffers
        /** @todo Additionally, OMX-CAMERA STEREO mode can't handle the rotation values! */
        if (m_capmode != VCAM_STEREO_MODE)
            VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_ROTATION, &m_camera_rotation, sizeof(uint32_t)));

        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->enablePreviewCbk(m_camcallback));
        DVP_PerformanceStart(&m_capPerf); // we'll time the camera init to first capture
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->sendCommand(VCAM_CMD_PREVIEW_START));
        //VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_DO_MANUALFOCUS, &m_focusDepth, sizeof(m_focusDepth)));

        if (AllocateNodes(2) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_VLIB_DISPARITY_SAD8;
            dvp_knode_to(&m_pNodes[0], DVP_Disparity_t)->left = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[0], DVP_Disparity_t)->right = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[0], DVP_Disparity_t)->disparity = m_images[m_dispMax+0];
            dvp_knode_to(&m_pNodes[0], DVP_Disparity_t)->mincost = m_images[m_dispMax+1];
            dvp_knode_to(&m_pNodes[0], DVP_Disparity_t)->cost = m_images[m_dispMax+2];
            dvp_knode_to(&m_pNodes[0], DVP_Disparity_t)->displacement = 10;

            // initialize the data in the nodes...
            m_pNodes[1].header.kernel = DVP_KN_NV12_TO_YUV444p;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[m_camIdx];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[m_dispMax+3];
            //m_pNodes[1].header.affinity = DVP_CORE_CPU;

            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        if (m_imgdbg_enabled && AllocateImageDebug(5))
        {
            ImageDebug_Init(&m_imgdbg[0], &m_images[m_camIdx], m_imgdbg_path, "01_input");
            ImageDebug_Init(&m_imgdbg[1], &m_images[m_dispMax+0], m_imgdbg_path, "02_disparity");
            ImageDebug_Init(&m_imgdbg[2], &m_images[m_dispMax+1], m_imgdbg_path, "03_mincost");
            ImageDebug_Init(&m_imgdbg[3], &m_images[m_dispMax+2], m_imgdbg_path, "04_cost");
            ImageDebug_Init(&m_imgdbg[4], &m_images[m_dispMax+3], m_imgdbg_path, "05_yuv444");
            ImageDebug_Open(m_imgdbg, m_numImgDbg);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_Tismo()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_TISMO)
    m_imgdbg_enabled = true_e;
    if (m_hDVP)
    {
        if (m_pCam == NULL)
        {
            m_pCam = VisionCamFactory(m_camtype);
            if (m_pCam == NULL)
                return STATUS_NOT_ENOUGH_MEMORY;
        }
#define TOPBOTTOM 1
#define DISPLAY_DISPARITY 0
#define FULL_SCREEN_DISPARITY 1
#if TOPBOTTOM
        VisionCamStereoInfo info = {VCAM_STEREO_LAYOUT_TOPBOTTOM, 1};
#else
        VisionCamStereoInfo info = {VCAM_STEREO_LAYOUT_LEFTRIGHT, 1};
#endif
        //VisionCamWhiteBalType white = VCAM_WHITE_BAL_CONTROL_AUTO;
        VisionCamResType res;
        uint32_t color = FOURCC_NV12; //FOURCC_Y800;//FOURCC_NV12;
        m_sensorIndex = VCAM_SENSOR_STEREO;
        m_capmode = VCAM_STEREO_MODE;
        //int brightness = 50;
        int32_t allocWidth;
        int32_t allocHeight;
        int32_t argWidth = m_width;
        int32_t argHeight = m_height;

        // Command line specifies width/height of each view.
        if (info.layout == VCAM_STEREO_LAYOUT_LEFTRIGHT && info.subsampling == 1) {
            m_width *= 2;
        } else if (info.layout == VCAM_STEREO_LAYOUT_TOPBOTTOM && info.subsampling == 1) {
            m_height *= 2;
        }

        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->init(this));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_WIDTH, &m_width, sizeof(uint32_t)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_HEIGHT, &m_height, sizeof(uint32_t)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_COLOR_SPACE_FOURCC, &color, sizeof(fourcc_t)));
        // Can't set ROTATION here, see below
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_FPS_FIXED, &m_fps, sizeof(uint32_t)));
        if (m_camtype == VISIONCAM_FILE)
        {
            VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_NAME, m_name, (int32_t)strlen(m_name)));
            VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_PATH, m_path, (int32_t)strlen(m_path)));
        }
        //VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_NAME, m_name, (int32_t)strlen(m_name)));
        //VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_PATH, m_path, (int32_t)strlen(m_path)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_SENSOR_SELECT, &m_sensorIndex, sizeof( m_sensorIndex) ));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_CAP_MODE, &m_capmode, sizeof(m_capmode)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_STEREO_INFO, &info, sizeof(info)));

        memset(&res, 0, sizeof(res));
        // VCAM_PARAM_2DBUFFER_DIM should only be called after resolutions, color space, cap mode, and
        //   optionally stereo information is set.
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->getParameter(VCAM_PARAM_2DBUFFER_DIM, &res, sizeof(res)));

#if TOPBOTTOM && DISPLAY_DISPARITY
        allocWidth = m_width*2;
#else
        allocWidth = res.mWidth;
#endif
        allocHeight = res.mHeight;

        m_camIdx = m_dispIdx = 0;
        m_camMin = m_dispMin = 0;
        if (m_camtype == VISIONCAM_FILE)
            m_camMax = m_dispMax = 1;  // Use one buffer to avoid frame dropping in file based mode
        else
            m_camMax = m_dispMax = 4;

        if (m_display_enabled)
#if TOPBOTTOM && DISPLAY_DISPARITY
#if FULL_SCREEN_DISPARITY
        m_display = DVP_Display_Create(m_width, m_height/2, allocWidth, allocHeight, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, 640, 480, 0, m_width, color, 0, m_dispMax+1);
#else
        m_display = DVP_Display_Create(m_width*2, m_height, allocWidth, allocHeight, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, allocWidth, allocHeight, 0, 0, color, 0, m_dispMax+1);
#endif
#else
        m_display = DVP_Display_Create(m_width, m_height, allocWidth, allocHeight, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, allocWidth, allocHeight, 0, 0, color, 0, m_dispMax+1);
#endif

        if (AllocateImageStructs(m_dispMax+3))
        {
            for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++)
            {
                DVP_Image_Init(&m_images[m_dispIdx], allocWidth, allocHeight, color);
                m_images[m_dispIdx].width = m_width;
                m_images[m_dispIdx].height = m_height;
            }

            DVP_Image_Init(&m_images[m_dispMax], argWidth, argHeight, FOURCC_Y16);   //Disparity Map
            DVP_Image_Init(&m_images[m_dispMax+1], argWidth, argHeight, FOURCC_Y800);   //Valid Codes
            DVP_Image_Init(&m_images[m_dispMax+2], argWidth, argHeight, FOURCC_NV12);   //Right

            if (m_display_enabled)
            {
                for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++) {
                    if (DVP_Display_Alloc(m_display, &m_images[m_dispIdx]) == DVP_FALSE)
                        return STATUS_NOT_ENOUGH_MEMORY;

                    for (DVP_U32 y = 0; y < m_images[m_dispIdx].bufHeight; y++) {
                        int i = (y * m_images[m_dispIdx].y_stride);
                        memset(&m_images[m_dispIdx].pData[0][i], 0, DVP_Image_LineSize(&m_images[m_dispIdx], 0));
                    }
                    for (DVP_U32 y = 0; y < m_images[m_dispIdx].bufHeight/2; y++) {
                        int i = (y * m_images[m_dispIdx].y_stride);
                        memset(&m_images[m_dispIdx].pData[1][i], 0x80, DVP_Image_LineSize(&m_images[m_dispIdx], 0));
                    }
                }
            }
            else {
                for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++)
                {
#if defined(DVP_USE_GRALLOC)
                    m_images[m_dispIdx].memType = DVP_MTYPE_GRALLOC_2DTILED;
#elif defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
                    m_images[m_dispIdx].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
#endif
                }
            }

            for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++) {
                if (DVP_Image_Alloc(m_hDVP, &m_images[m_dispIdx],(DVP_MemType_e)m_images[m_dispIdx].memType) == DVP_FALSE)
                    return STATUS_NOT_ENOUGH_MEMORY;
            }
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
            if (!DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+0], DVP_MTYPE_MPUNONCACHED_2DTILED) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+1], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+2], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
#else
            if (!DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+0], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+1], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+2], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
#endif
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->useBuffers(&m_images[m_camIdx], m_camMax));
        // @todo BUG: Can't set ROTATION until after useBuffers
        /** @todo Additionally, OMX-CAMERA STEREO mode can't handle the rotation values! */
        if (m_capmode != VCAM_STEREO_MODE)
            VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_ROTATION, &m_camera_rotation, sizeof(uint32_t)));

        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->enablePreviewCbk(m_camcallback));
        DVP_PerformanceStart(&m_capPerf); // we'll time the camera init to first capture
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->sendCommand(VCAM_CMD_PREVIEW_START));
        //VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_DO_MANUALFOCUS, &m_focusDepth, sizeof(m_focusDepth)));
        if (AllocateNodes(1) && AllocateGraphs(1) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
#ifdef DVP_USE_TISMO
            m_pNodes[0].header.kernel = DVP_KN_TISMO_DISPARITY;
#else
            m_pNodes[0].header.kernel = DVP_KN_DISPARITY_SAD8;
#endif
            // construct the left and right images from the camera input
            DVP_Image_t left = m_images[m_dispMin];
            DVP_Image_t right = m_images[m_dispMin];
            DVP_Image_t disp = m_images[m_dispMin];

            left.width = argWidth;
            left.height = argHeight;
            right.width = argWidth;
            right.height = argHeight;
            disp.width = argWidth;
            disp.height = argHeight;

           // y stride should be the same
            if (info.layout == VCAM_STEREO_LAYOUT_LEFTRIGHT && info.subsampling == 1) {
                right.pData[0] = right.pBuffer[0] + left.width*left.x_stride;
                disp = m_images[m_dispMax];
            } else if (info.layout == VCAM_STEREO_LAYOUT_TOPBOTTOM && info.subsampling == 1) {
                right.pData[0] = left.pBuffer[0] + left.height*left.y_stride;
#if DISPLAY_DISPARITY
                disp.pData[0]  = left.pBuffer[0] + left.width*left.x_stride;
                disp.pData[1]  = left.pBuffer[1] + left.width*left.x_stride;
#endif
            }

            //Copy right frame to separate buffer: WORKAROUND
            DVP_Image_Copy(&m_images[m_dispMax+2], &right);
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->left = left;
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->right = right; // m_images[m_dispMax+2];
#if DISPLAY_DISPARITY
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->output = disp;
#else
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->output = m_images[m_dispMax];
#endif
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->invalid = m_images[m_dispMax+1];
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->dMin = 40;
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->dMax = 80;
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->windowSize = 11;
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->consistency = 4;
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->uniqueness = 160;

            // put all the nodes in the section.
            m_correlation[0].portIdx = VCAM_PORT_PREVIEW;
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }

        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(5))
            {
                m_dbgImage = m_images[m_dispMin];
#if TOPBOTTOM
#if DISPLAY_DISPARITY
                m_dbgImage.width = argWidth * 2;
                m_dbgImage.y_stride *= 2;
#endif
                m_dbgImage.height = m_height;
#else
                m_dbgImage.width = m_width;
#endif
                ImageDebug_Init(&m_imgdbg[0], &m_dbgImage, m_imgdbg_path, "tismo");
                ImageDebug_Init(&m_imgdbg[1], &dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->output, m_imgdbg_path, "03_disparity");
                ImageDebug_Init(&m_imgdbg[2], &dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->invalid, m_imgdbg_path, "04_invalid");
                ImageDebug_Init(&m_imgdbg[3], &dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->left, m_imgdbg_path, "01_left");
                ImageDebug_Init(&m_imgdbg[4], &dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->right, m_imgdbg_path, "02_right");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_Tismov02()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_TISMOV02)
    m_imgdbg_enabled = true_e;
    if (m_hDVP)
    {
        if (m_pCam == NULL)
        {
            m_pCam = VisionCamFactory(m_camtype);
            if (m_pCam == NULL)
                return STATUS_NOT_ENOUGH_MEMORY;
        }
#define TOPBOTTOM 1
#define DISPLAY_DISPARITY 0
#define FULL_SCREEN_DISPARITY 1
#if TOPBOTTOM
        VisionCamStereoInfo info = {VCAM_STEREO_LAYOUT_TOPBOTTOM, 1};
#else
        VisionCamStereoInfo info = {VCAM_STEREO_LAYOUT_LEFTRIGHT, 1};
#endif
        //VisionCamWhiteBalType white = VCAM_WHITE_BAL_CONTROL_AUTO;
        VisionCamResType res;
        uint32_t color = FOURCC_NV12; //FOURCC_Y800;//FOURCC_NV12;
        m_sensorIndex = VCAM_SENSOR_STEREO;
        m_capmode = VCAM_STEREO_MODE;
        //int brightness = 50;
        int32_t allocWidth;
        int32_t allocHeight;
        int32_t argWidth = m_width;
        int32_t argHeight = m_height;

        // Command line specifies width/height of each view.
        if (info.layout == VCAM_STEREO_LAYOUT_LEFTRIGHT && info.subsampling == 1) {
            m_width *= 2;
        } else if (info.layout == VCAM_STEREO_LAYOUT_TOPBOTTOM && info.subsampling == 1) {
            m_height *= 2;
        }

        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->init(this));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_WIDTH, &m_width, sizeof(uint32_t)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_HEIGHT, &m_height, sizeof(uint32_t)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_COLOR_SPACE_FOURCC, &color, sizeof(fourcc_t)));
        // Can't set ROTATION here, see below
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_FPS_FIXED, &m_fps, sizeof(uint32_t)));
        if (m_camtype == VISIONCAM_FILE)
        {
            VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_NAME, m_name, (int32_t)strlen(m_name)));
            VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_PATH, m_path, (int32_t)strlen(m_path)));
        }
        //VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_NAME, m_name, (int32_t)strlen(m_name)));
        //VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_PATH, m_path, (int32_t)strlen(m_path)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_SENSOR_SELECT, &m_sensorIndex, sizeof( m_sensorIndex) ));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_CAP_MODE, &m_capmode, sizeof(m_capmode)));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_STEREO_INFO, &info, sizeof(info)));

        memset(&res, 0, sizeof(res));
        // VCAM_PARAM_2DBUFFER_DIM should only be called after resolutions, color space, cap mode, and
        //   optionally stereo information is set.
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->getParameter(VCAM_PARAM_2DBUFFER_DIM, &res, sizeof(res)));

#if TOPBOTTOM && DISPLAY_DISPARITY
        allocWidth = m_width*2;
#else
        allocWidth = res.mWidth;
#endif
        allocHeight = res.mHeight;

        m_camIdx = m_dispIdx = 0;
        m_camMin = m_dispMin = 0;
        if (m_camtype == VISIONCAM_FILE)
            m_camMax = m_dispMax = 1;  // Use one buffer to avoid frame dropping in file based mode
        else
            m_camMax = m_dispMax = 4;

        if (m_display_enabled)
#if TOPBOTTOM && DISPLAY_DISPARITY
#if FULL_SCREEN_DISPARITY
        m_display = DVP_Display_Create(m_width, m_height/2, allocWidth, allocHeight, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, 640, 480, 0, m_width, color, 0, m_dispMax+1);
#else
        m_display = DVP_Display_Create(m_width*2, m_height, allocWidth, allocHeight, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, allocWidth, allocHeight, 0, 0, color, 0, m_dispMax+1);
#endif
#else
        m_display = DVP_Display_Create(m_width, m_height, allocWidth, allocHeight, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, allocWidth, allocHeight, 0, 0, color, 0, m_dispMax+1);
#endif

        if (AllocateImageStructs(m_dispMax+9))
        {
            for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++)
            {
                DVP_Image_Init(&m_images[m_dispIdx], allocWidth, allocHeight, color);
                m_images[m_dispIdx].width = m_width;
                m_images[m_dispIdx].height = m_height;
            }

            DVP_Image_Init(&m_images[m_dispMax], argWidth, argHeight, FOURCC_Y800);   //out_lumaU8
            DVP_Image_Init(&m_images[m_dispMax+1], argWidth, argHeight, FOURCC_Y800);   //out_invalid
            DVP_Image_Init(&m_images[m_dispMax+2], argWidth, argHeight, FOURCC_Y16);   //out_raw
            DVP_Image_Init(&m_images[m_dispMax+3], argWidth, argHeight, FOURCC_RGBA);   //in_leftIntegralImage
            DVP_Image_Init(&m_images[m_dispMax+4], argWidth, argHeight, FOURCC_RGBA);   //in_rightIntegralImage
            DVP_Image_Init(&m_images[m_dispMax+5], argWidth, argHeight, FOURCC_UYVY);   //out_confidence
            DVP_Image_Init(&m_images[m_dispMax+6], argWidth, argHeight, FOURCC_UYVY);   //out_matchScore
            DVP_Image_Init(&m_images[m_dispMax+7], argWidth, argHeight, FOURCC_Y800);   //out_lumaU8
            DVP_Image_Init(&m_images[m_dispMax+8], argWidth, argHeight, FOURCC_Y800);   //out_invalid

            if (m_display_enabled)
            {
                for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++) {
                    if (DVP_Display_Alloc(m_display, &m_images[m_dispIdx]) == DVP_FALSE)
                        return STATUS_NOT_ENOUGH_MEMORY;

                    for (DVP_U32 y = 0; y < m_images[m_dispIdx].bufHeight; y++) {
                        int i = (y * m_images[m_dispIdx].y_stride);
                        memset(&m_images[m_dispIdx].pData[0][i], 0, m_images[m_dispIdx].bufWidth*m_images[m_dispIdx].x_stride);
                    }
                    for (DVP_U32 y = 0; y < m_images[m_dispIdx].bufHeight/2; y++) {
                        int i = (y * m_images[m_dispIdx].y_stride);
                        memset(&m_images[m_dispIdx].pData[1][i], 0x80, m_images[m_dispIdx].bufWidth*m_images[m_dispIdx].x_stride);
                    }
                }
            }
            else {
                for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++)
                {
#if defined(DVP_USE_GRALLOC)
                    m_images[m_dispIdx].memType = DVP_MTYPE_GRALLOC_2DTILED;
#elif defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
                    m_images[m_dispIdx].memType = DVP_MTYPE_MPUNONCACHED_2DTILED;
#endif
                }
            }

            for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++) {
                if (DVP_Image_Alloc(m_hDVP, &m_images[m_dispIdx],(DVP_MemType_e)m_images[m_dispIdx].memType) == DVP_FALSE)
                    return STATUS_NOT_ENOUGH_MEMORY;
            }
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
            if (!DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+0], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+1], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+2], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+3], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+4], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+5], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+6], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+7], DVP_MTYPE_DEFAULT) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[m_dispMax+8], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
#endif
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->useBuffers(&m_images[m_camIdx], m_camMax));
        // @todo BUG: Can't set ROTATION until after useBuffers
        /** @todo Additionally, OMX-CAMERA STEREO mode can't handle the rotation values! */
        if (m_capmode != VCAM_STEREO_MODE)
            VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_ROTATION, &m_camera_rotation, sizeof(uint32_t)));

        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->enablePreviewCbk(m_camcallback));
        DVP_PerformanceStart(&m_capPerf); // we'll time the camera init to first capture
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->sendCommand(VCAM_CMD_PREVIEW_START));
        //VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_DO_MANUALFOCUS, &m_focusDepth, sizeof(m_focusDepth)));
        if (AllocateNodes(3) && AllocateGraphs(1) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
#ifdef DVP_USE_TISMOV02
            m_pNodes[2].header.kernel = DVP_KN_TISMOV02_DISPARITY;//DVP_KN_TISMOV02_DISPARITY;
#else
            m_pNodes[2].header.kernel = DVP_KN_NOOP;
#endif
            // construct the left and right images from the camera input
            DVP_Image_t left = m_images[m_dispMin];
            DVP_Image_t right = m_images[m_dispMin];
            DVP_Image_t disp = m_images[m_dispMin];

            left.width = argWidth;
            left.height = argHeight;
            right.width = argWidth;
            right.height = argHeight;
            disp.width = argWidth;
            disp.height = argHeight;

           // y stride should be the same
            if (info.layout == VCAM_STEREO_LAYOUT_LEFTRIGHT && info.subsampling == 1) {
                right.pData[0] = right.pBuffer[0] +  left.width*left.x_stride;
                disp = m_images[m_dispMax];
            } else if (info.layout == VCAM_STEREO_LAYOUT_TOPBOTTOM && info.subsampling == 1) {
                right.pData[0] = left.pBuffer[0] + left.height*left.y_stride;
#if DISPLAY_DISPARITY
                disp.pData[0]  = left.pBuffer[0] + left.width*left.x_stride;
                disp.pData[1]  = left.pBuffer[1] + left.width*left.x_stride;
#endif
            }

            //Copy right frame to separate buffer: WORKAROUND
            DVP_Image_Copy(&m_images[m_dispMax+7], &left);
            DVP_Image_Copy(&m_images[m_dispMax+8], &right);
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->in_left = m_images[m_dispMax+7];
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->in_right = m_images[m_dispMax+8];

            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->in_leftIntegralImage = m_images[m_dispMax + 3];
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->in_rightIntegralImage = m_images[m_dispMax + 4];
#if DISPLAY_DISPARITY
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_falseColor_NV12 = disp;
#else
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_lumaU8 = m_images[m_dispMax];
#endif
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_invalid = m_images[m_dispMax+1];
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_raw = m_images[m_dispMax + 2];
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_confidence = m_images[m_dispMax + 5];
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_matchScore = m_images[m_dispMax + 6];

            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->disparityMin = 4;
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->disparityMax = 95;
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->windowSize = 11;
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->consistencyThreshold = 4;
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->n_fractionalBits = 3;//reset on the dsp side anyways
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->noiseFilterMethod = DVP_KN_TISMO_PERSEG_NF;//DVP_KN_TISMO_PERPIX_NF;
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->uniquenessThreshold = 160;//5000;// ->if NF is PER SEG and PF is on, then this needs to be larger
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->uniquenessMinValid = 1;
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->noiseThresholdDisparitySimilarity = 1;
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->prefilterMethod = DVP_KN_TISMO_PF_NONE;//DVP_KN_TISMO_PF_LOW_TEXTURE;//
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->planePrecision = DVP_KN_TISMO_PPP_OFF;//DVP_KN_TISMO_PPP_MEDIUM;//

            m_pNodes[0].header.kernel = DVP_KN_INTEGRAL_IMAGE_8;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[m_dispMax+7];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[m_dispMax + 3];
            m_pNodes[0].header.affinity = DVP_CORE_DSP;
            if(dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input.width> 2049 ||dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input.height> 819 )
               m_pNodes[0].header.affinity = DVP_CORE_CPU;

            m_pNodes[1].header.kernel = DVP_KN_INTEGRAL_IMAGE_8;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[m_dispMax+8];
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[m_dispMax + 4];
            m_pNodes[1].header.affinity = DVP_CORE_DSP;
            if(dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input.width> 2049 ||dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input.height> 819 )
               m_pNodes[1].header.affinity = DVP_CORE_CPU;

            // put all the nodes in the section.
            m_correlation[0].portIdx = VCAM_PORT_PREVIEW;
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }

        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(6))
            {
                m_dbgImage = m_images[m_dispMin];
#if TOPBOTTOM
#if DISPLAY_DISPARITY
                m_dbgImage.width = argWidth * 2;
                m_dbgImage.y_stride *= 2;
#endif
                m_dbgImage.height = m_height;
#else
                m_dbgImage.width = m_width;
#endif
                ImageDebug_Init(&m_imgdbg[0], &m_dbgImage, m_imgdbg_path, "tismov02");
                ImageDebug_Init(&m_imgdbg[1], &dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_lumaU8, m_imgdbg_path, "04_disparityU8");
                ImageDebug_Init(&m_imgdbg[2], &dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_invalid, m_imgdbg_path, "05_invalid");
                ImageDebug_Init(&m_imgdbg[3], &dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->in_left, m_imgdbg_path, "01_left");
                ImageDebug_Init(&m_imgdbg[4], &dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->in_right, m_imgdbg_path, "02_right");
                ImageDebug_Init(&m_imgdbg[5], &dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_raw, m_imgdbg_path, "03_disparityraw");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}


status_e TestVisionEngine::Test_Imglib()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_IMGLIB)
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_MPUNONCACHED_2DTILED;
    opType = DVP_MTYPE_MPUNONCACHED_1DTILED;
#endif
    DVP_S16 maskBlurNeg3x3[3][3] = {{-5, 20, 17} ,
                                    {19, -28, 13} ,
                                    {-22, 9, 33}};
    DVP_S16 maskBlur11x11[11][11] ={{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}};

    DVP_S08 maskBlur11x11_8[11][11] ={{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}};

    DVP_U08 maskSAD16x16[16][16] ={{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ,
                                    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}};
    if (m_hDVP)
    {
        if (AllocateImageStructs(72))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[6], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[7], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[8], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[9], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[10], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[11], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[12], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[13], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[14], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[15], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[16], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[17], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[18], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[19], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[20], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[21], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[22], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[23], 3, 3, FOURCC_Y16);
            DVP_Image_Init(&m_images[24], 5, 5, FOURCC_Y16);
            DVP_Image_Init(&m_images[25], 7, 7, FOURCC_Y16);
            DVP_Image_Init(&m_images[26], 11, 11, FOURCC_Y800);
            DVP_Image_Init(&m_images[27], 11, 11, FOURCC_Y16);
            DVP_Image_Init(&m_images[28], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[29], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[30], m_width, m_height, FOURCC_Y800); //conv 11x11 c8
            DVP_Image_Init(&m_images[31], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[32], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[33], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[34], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[35], m_width, m_height, FOURCC_YV16);
            DVP_Image_Init(&m_images[36], m_width, m_height, FOURCC_RGB565);
            DVP_Image_Init(&m_images[37], m_width, m_height, FOURCC_Y16); //clipping
            DVP_Image_Init(&m_images[38], m_width, m_height, FOURCC_Y32); //corr 3x3 c8
            DVP_Image_Init(&m_images[39], m_width, m_height, FOURCC_Y32); //corr 3x3 c16
            DVP_Image_Init(&m_images[40], m_width, m_height, FOURCC_Y32); //corr 3x3 i16
            DVP_Image_Init(&m_images[41], m_width, m_height, FOURCC_Y32); //corr 5x5 c16
            DVP_Image_Init(&m_images[42], m_width, m_height, FOURCC_Y32); //corr 11x11 c16
            DVP_Image_Init(&m_images[43], m_width, m_height, FOURCC_Y32); //corr 11x11 c16
            DVP_Image_Init(&m_images[44], m_width, m_height, FOURCC_Y16); //corr gen c16
            DVP_Image_Init(&m_images[45], m_width, m_height, FOURCC_Y32); //corr gen iq
            DVP_Image_Init(&m_images[46], m_width, m_height, FOURCC_Y800); //median 3x3
            DVP_Image_Init(&m_images[47], m_width, m_height, FOURCC_Y16); //median 3x3 16s
            DVP_Image_Init(&m_images[48], m_width, m_height, FOURCC_Y16); //expand
            DVP_Image_Init(&m_images[49], m_width, m_height, FOURCC_Y800); //saturate
            DVP_Image_Init(&m_images[50], m_width, m_height, FOURCC_Y16); //demuxbe Y
            DVP_Image_Init(&m_images[51], m_width/2, m_height, FOURCC_Y16); //demuxbe cr
            DVP_Image_Init(&m_images[52], m_width/2, m_height, FOURCC_Y16); //demuxbe cb
            DVP_Image_Init(&m_images[53], m_width, m_height, FOURCC_Y16); //demuxbe Y
            DVP_Image_Init(&m_images[54], m_width/2, m_height, FOURCC_Y16); //demuxbe cr
            DVP_Image_Init(&m_images[55], m_width/2, m_height, FOURCC_Y16); //demuxbe cb
            DVP_Image_Init(&m_images[56], m_width, m_height, FOURCC_Y800); //sad 8x8
            DVP_Image_Init(&m_images[57], m_width, m_height, FOURCC_Y800); //sad 16x16
            DVP_Image_Init(&m_images[58], 16, 16, FOURCC_Y800); //sad ref image
            DVP_Image_Init(&m_images[59], m_width, m_height, FOURCC_Y800); //sad 3x3
            DVP_Image_Init(&m_images[60], m_width, m_height, FOURCC_Y800); //sad 5x5
            DVP_Image_Init(&m_images[61], m_width, m_height, FOURCC_Y800); //sad 7x7
            DVP_Image_Init(&m_images[62], m_width, m_height, FOURCC_Y800); //err_diff 8
            DVP_Image_Init(&m_images[63], m_width, m_height, FOURCC_Y16); //err_diff 16
            DVP_Image_Init(&m_images[64], m_width+2, 1, FOURCC_Y16); //err_diff scratch
            DVP_Image_Init(&m_images[65], m_width, m_height, FOURCC_Y16); //fdct 8x8
            DVP_Image_Init(&m_images[66], m_width, m_height, FOURCC_Y16); //idct 8x8
            DVP_Image_Init(&m_images[67], m_width, m_height, FOURCC_RGBA); //mad 8x8
            DVP_Image_Init(&m_images[68], m_width, m_height, FOURCC_RGBA); //mad 16x16
            DVP_Image_Init(&m_images[69], m_width, m_height, FOURCC_Y16); // quantize
            DVP_Image_Init(&m_images[70], m_width, m_height, FOURCC_Y16); // wave_horz
            DVP_Image_Init(&m_images[71], m_width, m_height, FOURCC_Y16); // wave_vert
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], camType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType) || // DEFAULT is 1DTILED when using TILER
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[8], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[9], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[10], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[11], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[12], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[13], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[14], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[15], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[16], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[17], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[18], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[19], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[20], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[21], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[22], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[23], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[24], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[25], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[26], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[27], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[28], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[29], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[30], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[31], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[32], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[33], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[34], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[35], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[36], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[37], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[38], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[39], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[40], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[41], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[42], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[43], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[44], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[45], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[46], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[47], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[48], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[49], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[50], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[51], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[52], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[53], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[54], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[55], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[56], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[57], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[58], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[59], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[60], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[61], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[62], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[63], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[64], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[65], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[66], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[67], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[68], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[69], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[70], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[71], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateBufferStructs(7))
            {
                DVP_Buffer_Init(&m_buffers[0], 2, m_width*2);  // scratch conv 5x5 7x7
                DVP_Buffer_Init(&m_buffers[1], 2, 256); //img histogream 8
                DVP_Buffer_Init(&m_buffers[2], 2, (1<<16)); //img histogream 16
                DVP_Buffer_Init(&m_buffers[3], 2, (1<<16)*4); //max temp buf required
                DVP_Buffer_Init(&m_buffers[4], 2, 32); //quantize
                DVP_Buffer_Init(&m_buffers[5], 2, 8); //wavelet
                DVP_Buffer_Init(&m_buffers[6], 2, 8); //wavelet
                if (!DVP_Buffer_Alloc(m_hDVP, &m_buffers[0], DVP_MTYPE_DEFAULT) ||
                    !DVP_Buffer_Alloc(m_hDVP, &m_buffers[1], DVP_MTYPE_DEFAULT) ||
                    !DVP_Buffer_Alloc(m_hDVP, &m_buffers[2], DVP_MTYPE_DEFAULT) ||
                    !DVP_Buffer_Alloc(m_hDVP, &m_buffers[3], DVP_MTYPE_DEFAULT) ||
                    !DVP_Buffer_Alloc(m_hDVP, &m_buffers[4], DVP_MTYPE_DEFAULT) ||
                    !DVP_Buffer_Alloc(m_hDVP, &m_buffers[5], DVP_MTYPE_DEFAULT) ||
                    !DVP_Buffer_Alloc(m_hDVP, &m_buffers[6], DVP_MTYPE_DEFAULT))
                    return STATUS_NOT_ENOUGH_MEMORY;
            }
            else
                return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(61) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;      // Get Luma first
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];
            //m_pNodes[0].header.affinity = DVP_CORE_CPU;

            m_pNodes[1].header.kernel = DVP_KN_XSTRIDE_SHIFT;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[1];        // Use 8bit luma
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[2];       // Output 16bit luma
            //m_pNodes[1].header.affinity = DVP_CORE_CPU;

            m_pNodes[2].header.kernel = DVP_KN_IMG_THR_GT2MAX_8;
            dvp_knode_to(&m_pNodes[2], DVP_Threshold_t)->input = m_images[1];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[2], DVP_Threshold_t)->output = m_images[3];
            dvp_knode_to(&m_pNodes[2], DVP_Threshold_t)->thresh = 128;
            //m_pNodes[2].header.affinity = DVP_CORE_CPU;

            m_pNodes[3].header.kernel = DVP_KN_IMG_THR_GT2THR_8;
            dvp_knode_to(&m_pNodes[3], DVP_Threshold_t)->input = m_images[1];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[3], DVP_Threshold_t)->output = m_images[4];
            dvp_knode_to(&m_pNodes[3], DVP_Threshold_t)->thresh = 128;
            //m_pNodes[3].header.affinity = DVP_CORE_CPU;

            m_pNodes[4].header.kernel = DVP_KN_IMG_THR_LE2MIN_8;
            dvp_knode_to(&m_pNodes[4], DVP_Threshold_t)->input = m_images[1];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[4], DVP_Threshold_t)->output = m_images[5];
            dvp_knode_to(&m_pNodes[4], DVP_Threshold_t)->thresh = 128;
            //m_pNodes[4].header.affinity = DVP_CORE_CPU;

            m_pNodes[5].header.kernel = DVP_KN_IMG_THR_LE2THR_8;
            dvp_knode_to(&m_pNodes[5], DVP_Threshold_t)->input = m_images[1];    // Use 8bit luma
            dvp_knode_to(&m_pNodes[5], DVP_Threshold_t)->output = m_images[6];
            dvp_knode_to(&m_pNodes[5], DVP_Threshold_t)->thresh = 128;
            //m_pNodes[5].header.affinity = DVP_CORE_CPU;

            m_pNodes[6].header.kernel = DVP_KN_IMG_THR_GT2MAX_16;
            dvp_knode_to(&m_pNodes[6], DVP_Threshold_t)->input = m_images[2];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[6], DVP_Threshold_t)->output = m_images[7];
            dvp_knode_to(&m_pNodes[6], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;
            //m_pNodes[6].header.affinity = DVP_CORE_CPU;

            m_pNodes[7].header.kernel = DVP_KN_IMG_THR_GT2THR_16;
            dvp_knode_to(&m_pNodes[7], DVP_Threshold_t)->input = m_images[2];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[7], DVP_Threshold_t)->output = m_images[8];
            dvp_knode_to(&m_pNodes[7], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;
            //m_pNodes[7].header.affinity = DVP_CORE_CPU;

            m_pNodes[8].header.kernel = DVP_KN_IMG_THR_LE2MIN_16;
            dvp_knode_to(&m_pNodes[8], DVP_Threshold_t)->input = m_images[2];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[8], DVP_Threshold_t)->output = m_images[9];
            dvp_knode_to(&m_pNodes[8], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;
            //m_pNodes[8].header.affinity = DVP_CORE_CPU;

            m_pNodes[9].header.kernel = DVP_KN_IMG_THR_LE2THR_16;
            dvp_knode_to(&m_pNodes[9], DVP_Threshold_t)->input = m_images[2];    // Use 16bit luma
            dvp_knode_to(&m_pNodes[9], DVP_Threshold_t)->output = m_images[10];
            dvp_knode_to(&m_pNodes[9], DVP_Threshold_t)->thresh = (DVP_S16)0x8000;
            //m_pNodes[9].header.affinity = DVP_CORE_CPU;

            m_pNodes[10].header.kernel = DVP_KN_SOBEL_3x3_8s;
            dvp_knode_to(&m_pNodes[10], DVP_Transform_t)->input = m_images[1];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[10], DVP_Transform_t)->output = m_images[11];
            //m_pNodes[10].header.affinity = DVP_CORE_CPU;

            m_pNodes[11].header.kernel = DVP_KN_IMG_SOBEL_3x3_8;
            dvp_knode_to(&m_pNodes[11], DVP_Transform_t)->input = m_images[1];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[11], DVP_Transform_t)->output = m_images[12];
            //m_pNodes[11].header.affinity = DVP_CORE_CPU;

            m_pNodes[12].header.kernel = DVP_KN_SOBEL_5x5_8s;
            dvp_knode_to(&m_pNodes[12], DVP_Transform_t)->input = m_images[1];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[12], DVP_Transform_t)->output = m_images[13];
            //m_pNodes[12].header.affinity = DVP_CORE_CPU;

            m_pNodes[13].header.kernel = DVP_KN_SOBEL_5x5_8;
            dvp_knode_to(&m_pNodes[13], DVP_Transform_t)->input = m_images[1];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[13], DVP_Transform_t)->output = m_images[14];
            //m_pNodes[13].header.affinity = DVP_CORE_CPU;

            m_pNodes[14].header.kernel = DVP_KN_SOBEL_7x7_8s;
            dvp_knode_to(&m_pNodes[14], DVP_Transform_t)->input = m_images[1];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[14], DVP_Transform_t)->output = m_images[15];
            //m_pNodes[14].header.affinity = DVP_CORE_CPU;

            m_pNodes[15].header.kernel = DVP_KN_SOBEL_7x7_8;
            dvp_knode_to(&m_pNodes[15], DVP_Transform_t)->input = m_images[1];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[15], DVP_Transform_t)->output = m_images[16];
            //m_pNodes[15].header.affinity = DVP_CORE_CPU;

            m_pNodes[16].header.kernel = DVP_KN_IMG_SOBEL_3x3_16s;
            dvp_knode_to(&m_pNodes[16], DVP_Transform_t)->input = m_images[2];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[16], DVP_Transform_t)->output = m_images[17];
            //m_pNodes[16].header.affinity = DVP_CORE_CPU;

            m_pNodes[17].header.kernel = DVP_KN_SOBEL_3x3_16;
            dvp_knode_to(&m_pNodes[17], DVP_Transform_t)->input = m_images[2];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[17], DVP_Transform_t)->output = m_images[18];
            //m_pNodes[17].header.affinity = DVP_CORE_CPU;

            m_pNodes[18].header.kernel = DVP_KN_IMG_SOBEL_5x5_16s;
            dvp_knode_to(&m_pNodes[18], DVP_Transform_t)->input = m_images[2];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[18], DVP_Transform_t)->output = m_images[19];
            //m_pNodes[18].header.affinity = DVP_CORE_CPU;

            m_pNodes[19].header.kernel = DVP_KN_SOBEL_5x5_16;
            dvp_knode_to(&m_pNodes[19], DVP_Transform_t)->input = m_images[2];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[19], DVP_Transform_t)->output = m_images[20];
            //m_pNodes[19].header.affinity = DVP_CORE_CPU;

            m_pNodes[20].header.kernel = DVP_KN_IMG_SOBEL_7x7_16s;
            dvp_knode_to(&m_pNodes[20], DVP_Transform_t)->input = m_images[2];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[20], DVP_Transform_t)->output = m_images[21];
            //m_pNodes[20].header.affinity = DVP_CORE_CPU;

            m_pNodes[21].header.kernel = DVP_KN_SOBEL_7x7_16;
            dvp_knode_to(&m_pNodes[21], DVP_Transform_t)->input = m_images[2];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[21], DVP_Transform_t)->output = m_images[22];
            //m_pNodes[21].header.affinity = DVP_CORE_CPU;

            m_pNodes[22].header.kernel = DVP_KN_IMG_CONV_5x5_I8_C16;
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_with_buffer_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_with_buffer_t)->output = m_images[28];
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_with_buffer_t)->mask = m_images[24];
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_with_buffer_t)->scratch = m_buffers[0];
            dvp_knode_to(&m_pNodes[22], DVP_ImageConvolution_with_buffer_t)->shiftMask = 9;

            m_pNodes[23].header.kernel = DVP_KN_IMG_CONV_7x7_I8_C16;
            dvp_knode_to(&m_pNodes[23], DVP_ImageConvolution_with_buffer_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[23], DVP_ImageConvolution_with_buffer_t)->output = m_images[29];
            dvp_knode_to(&m_pNodes[23], DVP_ImageConvolution_with_buffer_t)->mask = m_images[25];
            dvp_knode_to(&m_pNodes[23], DVP_ImageConvolution_with_buffer_t)->scratch = m_buffers[0];
            dvp_knode_to(&m_pNodes[23], DVP_ImageConvolution_with_buffer_t)->shiftMask = 9;

            m_pNodes[24].header.kernel = DVP_KN_IMG_CONV_11x11;
            dvp_knode_to(&m_pNodes[24], DVP_ImageConvolution_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[24], DVP_ImageConvolution_t)->output = m_images[30];
            dvp_knode_to(&m_pNodes[24], DVP_ImageConvolution_t)->mask = m_images[26];
            dvp_knode_to(&m_pNodes[24], DVP_ImageConvolution_t)->shiftMask = 9;

            m_pNodes[25].header.kernel = DVP_KN_IMG_CONV_3x3_I16s_C16;
            dvp_knode_to(&m_pNodes[25], DVP_ImageConvolution_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[25], DVP_ImageConvolution_t)->output = m_images[31];
            dvp_knode_to(&m_pNodes[25], DVP_ImageConvolution_t)->mask = m_images[23];
            dvp_knode_to(&m_pNodes[25], DVP_ImageConvolution_t)->shiftMask = 13;

            m_pNodes[26].header.kernel = DVP_KN_IMG_CONV_5x5_I16s_C16;
            dvp_knode_to(&m_pNodes[26], DVP_ImageConvolution_with_buffer_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[26], DVP_ImageConvolution_with_buffer_t)->output = m_images[32];
            dvp_knode_to(&m_pNodes[26], DVP_ImageConvolution_with_buffer_t)->mask = m_images[24];
            dvp_knode_to(&m_pNodes[26], DVP_ImageConvolution_with_buffer_t)->shiftMask = 13;
            dvp_knode_to(&m_pNodes[26], DVP_ImageConvolution_with_buffer_t)->scratch = m_buffers[0];

            m_pNodes[27].header.kernel = DVP_KN_IMG_CONV_7x7_I16s_C16;
            dvp_knode_to(&m_pNodes[27], DVP_ImageConvolution_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[27], DVP_ImageConvolution_t)->output = m_images[33];
            dvp_knode_to(&m_pNodes[27], DVP_ImageConvolution_t)->mask = m_images[25];
            dvp_knode_to(&m_pNodes[27], DVP_ImageConvolution_t)->shiftMask = 13;

            m_pNodes[28].header.kernel = DVP_KN_IMG_CONV_11x11_I16s_C16;
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->output = m_images[34];
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->mask = m_images[27];
            dvp_knode_to(&m_pNodes[28], DVP_ImageConvolution_t)->shiftMask = 13;

            m_pNodes[29].header.kernel = DVP_KN_UYVY_TO_YUV422p;
            dvp_knode_to(&m_pNodes[29], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[29], DVP_Transform_t)->output = m_images[35];

            m_pNodes[30].header.kernel = DVP_KN_IMG_YUV422p_TO_RGB565;
            dvp_knode_to(&m_pNodes[30], DVP_Transform_t)->input = m_images[35];       // Use 16bit luma
            dvp_knode_to(&m_pNodes[30], DVP_Transform_t)->output = m_images[36];

            m_pNodes[31].header.kernel = DVP_KN_IMG_CLIPPING_16;
            dvp_knode_to(&m_pNodes[31], DVP_CannyHystThresholding_t)->inMag = m_images[2];
            dvp_knode_to(&m_pNodes[31], DVP_CannyHystThresholding_t)->output = m_images[37];
            dvp_knode_to(&m_pNodes[31], DVP_CannyHystThresholding_t)->hiThresh = 128;
            dvp_knode_to(&m_pNodes[31], DVP_CannyHystThresholding_t)->loThresh = 50;

            m_pNodes[32].header.kernel = DVP_KN_IMG_CORR_3x3;
            dvp_knode_to(&m_pNodes[32], DVP_ImageCorrelation_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[32], DVP_ImageCorrelation_t)->output = m_images[38];
            dvp_knode_to(&m_pNodes[32], DVP_ImageCorrelation_t)->mask = m_images[26];
            dvp_knode_to(&m_pNodes[32], DVP_ImageCorrelation_t)->shift = 7;
            dvp_knode_to(&m_pNodes[32], DVP_ImageCorrelation_t)->round = 64;

            m_pNodes[33].header.kernel = DVP_KN_IMG_CORR_3x3_I8_C16s;
            dvp_knode_to(&m_pNodes[33], DVP_ImageCorrelation_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[33], DVP_ImageCorrelation_t)->output = m_images[39];
            dvp_knode_to(&m_pNodes[33], DVP_ImageCorrelation_t)->mask = m_images[23];

            m_pNodes[34].header.kernel = DVP_KN_IMG_CORR_3x3_I16s_C16s;
            dvp_knode_to(&m_pNodes[34], DVP_ImageCorrelation_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[34], DVP_ImageCorrelation_t)->output = m_images[40];
            dvp_knode_to(&m_pNodes[34], DVP_ImageCorrelation_t)->mask = m_images[23];
            dvp_knode_to(&m_pNodes[34], DVP_ImageCorrelation_t)->shift = 6;
            dvp_knode_to(&m_pNodes[34], DVP_ImageCorrelation_t)->round = 1;

            m_pNodes[35].header.kernel = DVP_KN_IMG_CORR_5x5_I16s_C16s;
            dvp_knode_to(&m_pNodes[35], DVP_ImageCorrelation_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[35], DVP_ImageCorrelation_t)->output = m_images[41];
            dvp_knode_to(&m_pNodes[35], DVP_ImageCorrelation_t)->mask = m_images[24];
            dvp_knode_to(&m_pNodes[35], DVP_ImageCorrelation_t)->shift = 9;
            dvp_knode_to(&m_pNodes[35], DVP_ImageCorrelation_t)->round = 1;

            m_pNodes[36].header.kernel = DVP_KN_IMG_CORR_11x11_I8_C16s;
            dvp_knode_to(&m_pNodes[36], DVP_ImageCorrelation_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[36], DVP_ImageCorrelation_t)->output = m_images[42];
            dvp_knode_to(&m_pNodes[36], DVP_ImageCorrelation_t)->mask = m_images[27];

            m_pNodes[37].header.kernel = DVP_KN_IMG_CORR_11x11_I16s_C16s;
            dvp_knode_to(&m_pNodes[37], DVP_ImageCorrelation_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[37], DVP_ImageCorrelation_t)->output = m_images[43];
            dvp_knode_to(&m_pNodes[37], DVP_ImageCorrelation_t)->mask = m_images[27];
            dvp_knode_to(&m_pNodes[37], DVP_ImageCorrelation_t)->shift = 13;
            dvp_knode_to(&m_pNodes[37], DVP_ImageCorrelation_t)->round = 1;

            m_pNodes[38].header.kernel = DVP_KN_IMG_CORR_GEN_I16s_C16s;
            dvp_knode_to(&m_pNodes[38], DVP_ImageCorrelation_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[38], DVP_ImageCorrelation_t)->output = m_images[44];
            dvp_knode_to(&m_pNodes[38], DVP_ImageCorrelation_t)->mask = m_images[27];
            dvp_knode_to(&m_pNodes[38], DVP_ImageCorrelation_t)->shift = 13;
            dvp_knode_to(&m_pNodes[38], DVP_ImageCorrelation_t)->round = 1;
            dvp_knode_to(&m_pNodes[38], DVP_ImageCorrelation_t)->m = 11;

            m_pNodes[39].header.kernel = DVP_KN_IMG_CORR_GEN_IQ;
            dvp_knode_to(&m_pNodes[39], DVP_ImageCorrelation_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[39], DVP_ImageCorrelation_t)->output = m_images[45];
            dvp_knode_to(&m_pNodes[39], DVP_ImageCorrelation_t)->mask = m_images[27];
            dvp_knode_to(&m_pNodes[39], DVP_ImageCorrelation_t)->shift = 13;
            dvp_knode_to(&m_pNodes[39], DVP_ImageCorrelation_t)->round = 1;
            dvp_knode_to(&m_pNodes[39], DVP_ImageCorrelation_t)->m = 11;
            dvp_knode_to(&m_pNodes[39], DVP_ImageCorrelation_t)->x_qpt = 20;
            dvp_knode_to(&m_pNodes[39], DVP_ImageCorrelation_t)->h_qpt = 15;
            dvp_knode_to(&m_pNodes[39], DVP_ImageCorrelation_t)->y_qpt = 10;

            m_pNodes[40].header.kernel = DVP_KN_IMG_HISTOGRAM_8;
            dvp_knode_to(&m_pNodes[40], DVP_Histogram_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[40], DVP_Histogram_t)->histArray = m_buffers[3];
            dvp_knode_to(&m_pNodes[40], DVP_Histogram_t)->hOut = m_buffers[1];
            dvp_knode_to(&m_pNodes[40], DVP_Histogram_t)->binWeight = 1;

            m_pNodes[41].header.kernel = DVP_KN_IMG_HISTOGRAM_16;
            dvp_knode_to(&m_pNodes[41], DVP_Histogram_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[41], DVP_Histogram_t)->histArray = m_buffers[3];
            dvp_knode_to(&m_pNodes[41], DVP_Histogram_t)->hOut = m_buffers[2];
            dvp_knode_to(&m_pNodes[41], DVP_Histogram_t)->binWeight = 1;
            dvp_knode_to(&m_pNodes[41], DVP_Histogram_t)->numBins = 8;

            m_pNodes[42].header.kernel = DVP_KN_IMG_MEDIAN_3x3_8;
            dvp_knode_to(&m_pNodes[42], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[42], DVP_Transform_t)->output = m_images[46];

            m_pNodes[43].header.kernel = DVP_KN_IMG_MEDIAN_3x3_16s;
            dvp_knode_to(&m_pNodes[43], DVP_Transform_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[43], DVP_Transform_t)->output = m_images[47];

            m_pNodes[44].header.kernel = DVP_KN_IMG_PIX_EXPAND;
            dvp_knode_to(&m_pNodes[44], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[44], DVP_Transform_t)->output = m_images[48];

            m_pNodes[45].header.kernel = DVP_KN_IMG_PIX_SAT;
            dvp_knode_to(&m_pNodes[45], DVP_Transform_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[45], DVP_Transform_t)->output = m_images[49];

            m_pNodes[46].header.kernel = DVP_KN_IMG_YC_DEMUX_BE16_8;
            dvp_knode_to(&m_pNodes[46], DVP_Int2Pl_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[46], DVP_Int2Pl_t)->output1 = m_images[50];
            dvp_knode_to(&m_pNodes[46], DVP_Int2Pl_t)->output2 = m_images[51];
            dvp_knode_to(&m_pNodes[46], DVP_Int2Pl_t)->output3 = m_images[52];

            m_pNodes[47].header.kernel = DVP_KN_IMG_YC_DEMUX_LE16_8;
            dvp_knode_to(&m_pNodes[47], DVP_Int2Pl_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[47], DVP_Int2Pl_t)->output1 = m_images[53];
            dvp_knode_to(&m_pNodes[47], DVP_Int2Pl_t)->output2 = m_images[54];
            dvp_knode_to(&m_pNodes[47], DVP_Int2Pl_t)->output3 = m_images[55];

            if((m_testcore == DVP_CORE_DSP) || (m_testcore ==DVP_CORE_CPU))
                m_pNodes[48].header.kernel = DVP_KN_IMG_SAD_8x8;
#if defined(DVP_USE_VRUN)
            else
                m_pNodes[48].header.kernel = DVP_KN_VRUN_SAD_8x8;
#endif
            dvp_knode_to(&m_pNodes[48], DVP_SAD_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[48], DVP_SAD_t)->output = m_images[56];
            dvp_knode_to(&m_pNodes[48], DVP_SAD_t)->refImg = m_images[58];
            dvp_knode_to(&m_pNodes[48], DVP_SAD_t)->shiftMask = 4;
            dvp_knode_to(&m_pNodes[48], DVP_SAD_t)->refPitch = 8;
            dvp_knode_to(&m_pNodes[48], DVP_SAD_t)->refStartOffset = 0;

            if((m_testcore == DVP_CORE_DSP) || (m_testcore ==DVP_CORE_CPU))
                m_pNodes[49].header.kernel = DVP_KN_IMG_SAD_16x16;
#if defined(DVP_USE_VRUN)
            else
                m_pNodes[49].header.kernel = DVP_KN_VRUN_SAD_16x16;
#endif
            dvp_knode_to(&m_pNodes[49], DVP_SAD_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[49], DVP_SAD_t)->output = m_images[57];
            dvp_knode_to(&m_pNodes[49], DVP_SAD_t)->refImg = m_images[58];
            dvp_knode_to(&m_pNodes[49], DVP_SAD_t)->shiftMask = 8;
            dvp_knode_to(&m_pNodes[49], DVP_SAD_t)->refPitch = 16;
            dvp_knode_to(&m_pNodes[49], DVP_SAD_t)->refStartOffset = 0;

            if((m_testcore == DVP_CORE_DSP) || (m_testcore ==DVP_CORE_CPU))
                m_pNodes[50].header.kernel = DVP_KN_IMG_SAD_3x3;
#if defined(DVP_USE_VRUN)
            else
                m_pNodes[50].header.kernel = DVP_KN_VRUN_SAD_3x3;
#endif
            dvp_knode_to(&m_pNodes[50], DVP_SAD_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[50], DVP_SAD_t)->output = m_images[59];
            dvp_knode_to(&m_pNodes[50], DVP_SAD_t)->refImg = m_images[58];
            dvp_knode_to(&m_pNodes[50], DVP_SAD_t)->refPitch = 3;
            dvp_knode_to(&m_pNodes[50], DVP_SAD_t)->refStartOffset = 0;

            if((m_testcore == DVP_CORE_DSP) || (m_testcore ==DVP_CORE_CPU))
                m_pNodes[51].header.kernel = DVP_KN_IMG_SAD_5x5;
#if defined(DVP_USE_VRUN)
            else
                m_pNodes[51].header.kernel = DVP_KN_VRUN_SAD_5x5;
#endif
            dvp_knode_to(&m_pNodes[51], DVP_SAD_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[51], DVP_SAD_t)->output = m_images[60];
            dvp_knode_to(&m_pNodes[51], DVP_SAD_t)->refImg = m_images[58];
            dvp_knode_to(&m_pNodes[51], DVP_SAD_t)->refPitch = 5;
            dvp_knode_to(&m_pNodes[51], DVP_SAD_t)->refStartOffset = 0;

            if((m_testcore == DVP_CORE_DSP) || (m_testcore ==DVP_CORE_CPU))
                m_pNodes[52].header.kernel = DVP_KN_IMG_SAD_7x7;
#if defined(DVP_USE_VRUN)
            else
                m_pNodes[52].header.kernel = DVP_KN_VRUN_SAD_7x7;
#endif
            dvp_knode_to(&m_pNodes[52], DVP_SAD_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[52], DVP_SAD_t)->output = m_images[61];
            dvp_knode_to(&m_pNodes[52], DVP_SAD_t)->refImg = m_images[58];
            dvp_knode_to(&m_pNodes[52], DVP_SAD_t)->refPitch = 7;
            dvp_knode_to(&m_pNodes[52], DVP_SAD_t)->refStartOffset = 0;

            m_pNodes[53].header.kernel = DVP_KN_IMG_ERRDIFF_BIN_8;
            dvp_knode_to(&m_pNodes[53], DVP_Threshold_t)->input = m_images[62];
            dvp_knode_to(&m_pNodes[53], DVP_Threshold_t)->output = m_images[64]; //scratch
            dvp_knode_to(&m_pNodes[53], DVP_Threshold_t)->thresh = 100;

            m_pNodes[54].header.kernel = DVP_KN_IMG_ERRDIFF_BIN_16;
            dvp_knode_to(&m_pNodes[54], DVP_Threshold_t)->input = m_images[63];
            dvp_knode_to(&m_pNodes[54], DVP_Threshold_t)->output = m_images[64]; //scratch
            dvp_knode_to(&m_pNodes[54], DVP_Threshold_t)->thresh = 100;

            m_pNodes[55].header.kernel = DVP_KN_IMG_FDCT_8x8;
            dvp_knode_to(&m_pNodes[55], DVP_Transform_t)->input = m_images[65];

            m_pNodes[56].header.kernel = DVP_KN_IMG_IDCT_8x8_12Q4;
            dvp_knode_to(&m_pNodes[56], DVP_Transform_t)->input = m_images[66];

            m_pNodes[57].header.kernel = DVP_KN_IMG_MAD_8x8;
            dvp_knode_to(&m_pNodes[57], DVP_SAD_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[57], DVP_SAD_t)->output = m_images[67];
            dvp_knode_to(&m_pNodes[57], DVP_SAD_t)->refImg = m_images[58];

            m_pNodes[58].header.kernel = DVP_KN_IMG_MAD_16x16;
            dvp_knode_to(&m_pNodes[58], DVP_SAD_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[58], DVP_SAD_t)->output = m_images[68];
            dvp_knode_to(&m_pNodes[58], DVP_SAD_t)->refImg = m_images[58];

            m_pNodes[59].header.kernel = DVP_KN_IMG_QUANTIZE_16;
            dvp_knode_to(&m_pNodes[59], DVP_ImageQuantization_t)->input = m_images[69]; //in-place operaition
            dvp_knode_to(&m_pNodes[59], DVP_ImageQuantization_t)->recip_tbl = m_buffers[4];
            dvp_knode_to(&m_pNodes[59], DVP_ImageQuantization_t)->block_size = 32;
            dvp_knode_to(&m_pNodes[59], DVP_ImageQuantization_t)->q_pt = 3;

            m_pNodes[60].header.kernel = DVP_KN_IMG_WAVE_HORZ;
            dvp_knode_to(&m_pNodes[60], DVP_ImageWavelet_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[60], DVP_ImageWavelet_t)->output = m_images[70];
            dvp_knode_to(&m_pNodes[60], DVP_ImageWavelet_t)->qmf = m_buffers[5];
            dvp_knode_to(&m_pNodes[60], DVP_ImageWavelet_t)->mqmf = m_buffers[6];

            memcpy(m_images[23].pData[0], maskBlurNeg3x3, 3*3*sizeof(DVP_S16));
            memcpy(m_images[24].pData[0], maskBlur11x11,  5*5*sizeof(DVP_S16));
            memcpy(m_images[25].pData[0], maskBlur11x11,  7*7*sizeof(DVP_S16));
            memcpy(m_images[26].pData[0], maskBlur11x11_8, 11*11*sizeof(DVP_U08));
            memcpy(m_images[27].pData[0], maskBlur11x11,  11*11*sizeof(DVP_S16));
            memcpy(m_images[58].pData[0], maskSAD16x16,  16*16*sizeof(DVP_U08));

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }

        if(m_testcore != DVP_CORE_MIN) {
            for(uint32_t testnode=0; testnode<m_numNodes; testnode++) {
                m_pNodes[testnode].header.affinity = m_testcore;
            }
        }

        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(67))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_luma16");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "03_thr8_gtMAX");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "04_thr8_gtTHR");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "05_thr8_leMIN");
                ImageDebug_Init(&m_imgdbg[6], &m_images[6], m_imgdbg_path, "06_thr8_leTHR");
                ImageDebug_Init(&m_imgdbg[7], &m_images[7], m_imgdbg_path, "07_thr16_gtMAX");
                ImageDebug_Init(&m_imgdbg[8], &m_images[8], m_imgdbg_path, "08_thr16_gtTHR");
                ImageDebug_Init(&m_imgdbg[9], &m_images[9], m_imgdbg_path, "09_thr16_leMIN");
                ImageDebug_Init(&m_imgdbg[10], &m_images[10], m_imgdbg_path, "10_thr16_leTHR");
                ImageDebug_Init(&m_imgdbg[11], &m_images[11], m_imgdbg_path, "11_sobel8_3X3_s");
                ImageDebug_Init(&m_imgdbg[12], &m_images[12], m_imgdbg_path, "12_sobel8_3X3");
                ImageDebug_Init(&m_imgdbg[13], &m_images[13], m_imgdbg_path, "13_sobel8_5X5_s");
                ImageDebug_Init(&m_imgdbg[14], &m_images[14], m_imgdbg_path, "14_sobel8_5X5");
                ImageDebug_Init(&m_imgdbg[15], &m_images[15], m_imgdbg_path, "15_sobel8_7X7_s");
                ImageDebug_Init(&m_imgdbg[16], &m_images[16], m_imgdbg_path, "16_sobel8_7X7");
                ImageDebug_Init(&m_imgdbg[17], &m_images[17], m_imgdbg_path, "17_sobel16_3X3_s");
                ImageDebug_Init(&m_imgdbg[18], &m_images[18], m_imgdbg_path, "18_sobel16_3X3");
                ImageDebug_Init(&m_imgdbg[19], &m_images[19], m_imgdbg_path, "19_sobel16_5X5_s");
                ImageDebug_Init(&m_imgdbg[20], &m_images[20], m_imgdbg_path, "20_sobel16_5X5");
                ImageDebug_Init(&m_imgdbg[21], &m_images[21], m_imgdbg_path, "21_sobel16_7X7_s");
                ImageDebug_Init(&m_imgdbg[22], &m_images[22], m_imgdbg_path, "22_sobel16_7X7");
                ImageDebug_Init(&m_imgdbg[23], &m_images[28], m_imgdbg_path, "23_convi8c16_5X5");
                ImageDebug_Init(&m_imgdbg[24], &m_images[29], m_imgdbg_path, "24_convi8c16_7X7");
                ImageDebug_Init(&m_imgdbg[25], &m_images[30], m_imgdbg_path, "25_conv8_11X11");
                ImageDebug_Init(&m_imgdbg[26], &m_images[31], m_imgdbg_path, "26_convi16c16_3X3");
                ImageDebug_Init(&m_imgdbg[27], &m_images[32], m_imgdbg_path, "27_convi16c16_5X5");
                ImageDebug_Init(&m_imgdbg[28], &m_images[33], m_imgdbg_path, "28_convi16c16_7X7");
                ImageDebug_Init(&m_imgdbg[29], &m_images[34], m_imgdbg_path, "29_convi16c16_11X11");
                ImageDebug_Init(&m_imgdbg[30], &m_images[35], m_imgdbg_path, "30_yuv422pl");
                ImageDebug_Init(&m_imgdbg[31], &m_images[36], m_imgdbg_path, "31_yuv2rgb565");
                ImageDebug_Init(&m_imgdbg[32], &m_images[37], m_imgdbg_path, "32_clipping");
                ImageDebug_Init(&m_imgdbg[33], &m_images[38], m_imgdbg_path, "35_corr3X3i8");
                ImageDebug_Init(&m_imgdbg[34], &m_images[39], m_imgdbg_path, "36_corr3X3c16");
                ImageDebug_Init(&m_imgdbg[35], &m_images[40], m_imgdbg_path, "37_corr3X3i16");
                ImageDebug_Init(&m_imgdbg[36], &m_images[41], m_imgdbg_path, "38_corr5X5i8");
                ImageDebug_Init(&m_imgdbg[37], &m_images[42], m_imgdbg_path, "39_corr11X11i8");
                ImageDebug_Init(&m_imgdbg[38], &m_images[43], m_imgdbg_path, "40_corr11X11i16");
                ImageDebug_Init(&m_imgdbg[39], &m_images[44], m_imgdbg_path, "41_corrGeni16");
                ImageDebug_Init(&m_imgdbg[40], &m_images[45], m_imgdbg_path, "42_corrGeniq");
                ImageDebug_Init(&m_imgdbg[41], &m_images[46], m_imgdbg_path, "43_median3X3_8");
                ImageDebug_Init(&m_imgdbg[42], &m_images[47], m_imgdbg_path, "44_median3X3_16s");
                ImageDebug_Init(&m_imgdbg[43], &m_images[48], m_imgdbg_path, "47_piXEXpand");
                ImageDebug_Init(&m_imgdbg[44], &m_images[49], m_imgdbg_path, "48_piXSat");
                ImageDebug_Init(&m_imgdbg[45], &m_images[50], m_imgdbg_path, "49_demuXBEY");
                ImageDebug_Init(&m_imgdbg[46], &m_images[51], m_imgdbg_path, "49_demuXBECr");
                ImageDebug_Init(&m_imgdbg[47], &m_images[52], m_imgdbg_path, "49_demuXBECb");
                ImageDebug_Init(&m_imgdbg[48], &m_images[53], m_imgdbg_path, "50_demuXLEY");
                ImageDebug_Init(&m_imgdbg[49], &m_images[54], m_imgdbg_path, "50_demuXLECr");
                ImageDebug_Init(&m_imgdbg[50], &m_images[55], m_imgdbg_path, "50_demuXLECb");
                ImageDebug_Init(&m_imgdbg[53], &m_images[56], m_imgdbg_path, "53_SAD_8X8");
                ImageDebug_Init(&m_imgdbg[54], &m_images[57], m_imgdbg_path, "54_SAD_16X16");
                ImageDebug_Init(&m_imgdbg[55], &m_images[59], m_imgdbg_path, "55_SAD_3X3");
                ImageDebug_Init(&m_imgdbg[56], &m_images[60], m_imgdbg_path, "56_SAD_5X5");
                ImageDebug_Init(&m_imgdbg[57], &m_images[61], m_imgdbg_path, "57_SAD_7X7");
                ImageDebug_Init(&m_imgdbg[58], &m_images[62], m_imgdbg_path, "58_errDiff8");
                ImageDebug_Init(&m_imgdbg[59], &m_images[63], m_imgdbg_path, "59_errDiff16");
                ImageDebug_Init(&m_imgdbg[60], &m_images[65], m_imgdbg_path, "60_fdct8X8");
                ImageDebug_Init(&m_imgdbg[61], &m_images[66], m_imgdbg_path, "61_idct8X8");
                ImageDebug_Init(&m_imgdbg[62], &m_images[67], m_imgdbg_path, "62_mad8X8");
                ImageDebug_Init(&m_imgdbg[63], &m_images[68], m_imgdbg_path, "63_mad16X16");
                ImageDebug_Init(&m_imgdbg[64], &m_images[69], m_imgdbg_path, "64_quant16");
                ImageDebug_Init(&m_imgdbg[65], &m_images[70], m_imgdbg_path, "65_waveHorz");
                ImageDebug_Init(&m_imgdbg[66], &m_images[71], m_imgdbg_path, "66_waveVert");
                BufferDebug_Init(&m_imgdbg[51], &m_buffers[1], m_imgdbg_path, "51_ImgHist8", ".bw");
                BufferDebug_Init(&m_imgdbg[52], &m_buffers[2], m_imgdbg_path, "52_ImgHist16", ".bw");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_Ldc()
{
    status_e status = STATUS_SUCCESS;
    bool_e inputImagePadding = true_e;
    DVP_Image_t tmpImages[1];
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;

    DVP_U16 lut[256] = {
        9300,9289,9278,9267,9256,9245,9234,9223,9212,9201,9190,9179,9168,9157,9146,9135,
        9125,9114,9103,9092,9081,9070,9059,9048,9037,9026,9015,9004,8993,8982,8971,8960,
        8950,8936,8922,8909,8895,8882,8868,8854,8841,8827,8814,8800,8786,8773,8759,8746,
        8732,8718,8705,8691,8678,8664,8650,8637,8623,8610,8596,8582,8569,8555,8542,8528,
        8515,8500,8486,8471,8457,8442,8428,8413,8399,8384,8370,8355,8341,8326,8312,8297,
        8283,8269,8254,8240,8225,8211,8196,8182,8167,8153,8138,8124,8109,8095,8080,8066,
        8052,8039,8026,8014,8001,7989,7976,7964,7951,7938,7926,7913,7901,7888,7876,7863,
        7851,7838,7825,7813,7800,7788,7775,7763,7750,7737,7725,7712,7700,7687,7675,7662,
        7650,7643,7636,7629,7622,7615,7608,7601,7595,7588,7581,7574,7567,7560,7553,7546,
        7540,7533,7526,7519,7512,7505,7498,7491,7485,7478,7471,7464,7457,7450,7443,7436,
        7430,7422,7414,7406,7398,7390,7383,7375,7367,7359,7351,7344,7336,7328,7320,7312,
        7305,7297,7289,7281,7273,7265,7258,7250,7242,7234,7226,7219,7211,7203,7195,7187,
        7180,7178,7176,7174,7172,7170,7168,7166,7165,7163,7161,7159,7157,7155,7153,7151,
        7150,7148,7146,7144,7142,7140,7138,7136,7135,7133,7131,7129,7127,7125,7123,7121,
        7120,7110,7100,7090,7080,7070,7060,7050,7040,7030,7020,7010,7000,6990,6980,6970,
        6960,6950,6940,6930,6920,6910,6900,6890,6880,6870,6860,6850,6840,6830,6820, 0};

#if defined(DVP_USE_ION) || defined(DVP_USE_TILER) || defined(DVP_USE_BO)
    opType = DVP_MTYPE_MPUCACHED_1DTILED;
#endif

#if defined(DVP_USE_VRUN)
    if (m_hDVP)
    {
        if (AllocateImageStructs(8))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_NV12);
            DVP_Image_Init(&tmpImages[0], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_UYVY); // To see full output of data, need larger output size
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_NV12);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_NV12);
            DVP_Image_Init(&m_images[6], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[7], m_width, m_height, FOURCC_NV12);

            if(inputImagePadding) {         // This is required to avoid trash data outside of input range.
                m_images[0].bufHeight *= 2;
                m_images[0].bufWidth *= 2;
                m_images[1].bufHeight *= 2;
                m_images[1].bufWidth *= 2;
            }

            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], opType))
                return STATUS_NOT_ENOUGH_MEMORY;

            if(inputImagePadding) { // Moving the image to middle of buffer
                DVP_Image_SetPatch(&m_images[0], 0, m_height/2, m_width, m_height);
                DVP_Image_SetPatch(&m_images[1], 0, m_height/2, m_width, m_height);
            }
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        DVP_PRINT(DVP_ZONE_ALWAYS, "I am here\n");

        if (AllocateBufferStructs(1))
        {
            DVP_Buffer_Init(&m_buffers[0], 2, 256);  // LDC lut
            if (!DVP_Buffer_Alloc(m_hDVP, &m_buffers[0], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(7) && AllocateSections(&m_graphs[0], 1))
        {
            int angleDegrees = 15;
            double angleRadians = angleDegrees*M_PI/180;
            unsigned short interpolateLuma = 0;

            // Center of rotation in input
            int x0in  = m_images[0].width/2;
            int y0in  = m_images[0].height/2;

            // Cooresponding center of rotation in output
            int x0out = m_images[1].width/2;
            int y0out = m_images[1].height/2;

            tmpImages[0].pBuffer[0] = m_images[1].pBuffer[0];    // Using Luma from luma extract function
            tmpImages[0].pData[0] = m_images[1].pData[0];
            tmpImages[0].y_stride = m_images[1].y_stride;
            tmpImages[0].y_start  = m_images[1].y_start;
            tmpImages[0].memType  = m_images[1].memType;

            /* Luma Extraction */
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = tmpImages[0];

            /* YUV422 Image Rotation */
            m_pNodes[1].header.kernel = DVP_KN_LDC_AFFINE_TRANSFORM;
            dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->input  = m_images[0];
            dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->output = m_images[2];
            dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->pixpad = 6;
            dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->interpolationLuma = interpolateLuma;

            dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[0] = (DVP_S16)(4096*cos(angleRadians));
            dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[1] = (DVP_S16)(4096*sin(angleRadians));
            dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[2] = 8*x0in-
                                                8*dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[0]*x0out/4096-
                                                8*dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[1]*y0out/4096;
            dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[3] = (DVP_S16)(-4096*sin(angleRadians));
            dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[4] = (DVP_S16)(4096*cos(angleRadians));
            dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[5] = 8*y0in-
                                                8*dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[3]*x0out/4096-
                                                8*dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[4]*y0out/4096;

            for(int i=0; i<6; i++)
                DVP_PRINT(DVP_ZONE_ALWAYS, "Affine[%d]: %d\n", i, dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[i]);

            /* NV12 Image Rotation */
            m_pNodes[2].header.kernel = DVP_KN_LDC_AFFINE_TRANSFORM;
            dvp_knode_to(&m_pNodes[2], DVP_Ldc_t)->input  = m_images[1];
            dvp_knode_to(&m_pNodes[2], DVP_Ldc_t)->output = m_images[3];
            dvp_knode_to(&m_pNodes[2], DVP_Ldc_t)->pixpad = 6;
            dvp_knode_to(&m_pNodes[2], DVP_Ldc_t)->interpolationLuma = interpolateLuma;

            dvp_knode_to(&m_pNodes[2], DVP_Ldc_t)->affine[0] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[0];
            dvp_knode_to(&m_pNodes[2], DVP_Ldc_t)->affine[1] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[1];
            dvp_knode_to(&m_pNodes[2], DVP_Ldc_t)->affine[2] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[2];
            dvp_knode_to(&m_pNodes[2], DVP_Ldc_t)->affine[3] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[3];
            dvp_knode_to(&m_pNodes[2], DVP_Ldc_t)->affine[4] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[4];
            dvp_knode_to(&m_pNodes[2], DVP_Ldc_t)->affine[5] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[5];

            memcpy(m_buffers[0].pData, lut, 256*2);

            /* YUV422 Distortion Correction */
            m_pNodes[3].header.kernel = DVP_KN_LDC_DISTORTION_CORRECTION;
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->input  = m_images[0];
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->output = m_images[4];
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->pixpad = 3;
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->interpolationLuma = interpolateLuma;

            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->ldcLut  = m_buffers[0];
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->ldcLensCenterX  = m_width/2;
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->ldcLensCenterY  = m_height/2;
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->ldcKhl  = 209;
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->ldcKhr  = 190;
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->ldcKvu  = 190;
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->ldcKvl  = 190;
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->ldcRth  = 0;
            dvp_knode_to(&m_pNodes[3], DVP_Ldc_t)->ldcRightShiftBits  = 11;

            /* NV12 Distortion Correction */
            m_pNodes[4].header.kernel = DVP_KN_LDC_DISTORTION_CORRECTION;
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->input  = m_images[1];
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->output = m_images[5];
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->pixpad = 3;
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->interpolationLuma = interpolateLuma;

            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->ldcLut  = m_buffers[0];
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->ldcLensCenterX  = m_width/2;
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->ldcLensCenterY  = m_height/2;
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->ldcKhl  = 209;
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->ldcKhr  = 190;
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->ldcKvu  = 190;
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->ldcKvl  = 190;
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->ldcRth  = 0;
            dvp_knode_to(&m_pNodes[4], DVP_Ldc_t)->ldcRightShiftBits  = 11;

            /* YUV422 Distortion Correction + Affine rotation */
            m_pNodes[5].header.kernel = DVP_KN_LDC_DISTORTION_AND_AFFINE;
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->input  = m_images[0];
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->output = m_images[6];
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->pixpad = 3;
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->interpolationLuma = interpolateLuma;

            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->affine[0] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[0];
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->affine[1] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[1];
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->affine[2] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[2];
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->affine[3] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[3];
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->affine[4] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[4];
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->affine[5] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[5];

            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->ldcLut  = m_buffers[0];
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->ldcLensCenterX  = m_width/2;
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->ldcLensCenterY  = m_height/2;
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->ldcKhl  = 209;
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->ldcKhr  = 190;
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->ldcKvu  = 190;
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->ldcKvl  = 190;
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->ldcRth  = 0;
            dvp_knode_to(&m_pNodes[5], DVP_Ldc_t)->ldcRightShiftBits  = 11;

            /* NV12 Distortion Correction + Affine rotation */
            m_pNodes[6].header.kernel = DVP_KN_LDC_DISTORTION_AND_AFFINE;
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->input  = m_images[1];
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->output = m_images[7];
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->pixpad = 3;
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->interpolationLuma = interpolateLuma;

            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->affine[0] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[0];
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->affine[1] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[1];
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->affine[2] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[2];
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->affine[3] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[3];
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->affine[4] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[4];
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->affine[5] = dvp_knode_to(&m_pNodes[1], DVP_Ldc_t)->affine[5];

            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->ldcLut  = m_buffers[0];
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->ldcLensCenterX  = m_width/2;
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->ldcLensCenterY  = m_height/2;
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->ldcKhl  = 209;
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->ldcKhr  = 190;
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->ldcKvu  = 190;
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->ldcKvl  = 190;
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->ldcRth  = 0;
            dvp_knode_to(&m_pNodes[6], DVP_Ldc_t)->ldcRightShiftBits  = 11;

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(8))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_422_affine");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "03_nv12_affine");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "04_422_dist");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "05_nv12_dist");
                ImageDebug_Init(&m_imgdbg[6], &m_images[6], m_imgdbg_path, "06_422_dist_affine");
                ImageDebug_Init(&m_imgdbg[7], &m_images[7], m_imgdbg_path, "07_nv12_dist_affine");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_AR()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VLIB) || defined(DVP_USE_VRUN)
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;

#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_MPUNONCACHED_2DTILED;
    opType = DVP_MTYPE_MPUCACHED_1DTILED;
#endif
#define NPLUS1  4
    if (m_hDVP)
    {
        if (AllocateImageStructs(7))
        {
            short new_m_width = ((m_width/NPLUS1) % 16)?((m_width/NPLUS1) + 15 ) & 0xFFFF0 : (m_width/NPLUS1); //next multiple of 16
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[3], new_m_width, m_height/NPLUS1, FOURCC_Y16);
            DVP_Image_Init(&m_images[4], new_m_width, m_height/NPLUS1, FOURCC_Y16);
            DVP_Image_Init(&m_images[5], new_m_width, m_height/NPLUS1, FOURCC_Y16);
            DVP_Image_Init(&m_images[6], new_m_width, m_height/NPLUS1, FOURCC_Y800);
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], camType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(4) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;      // Get Luma first
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input  = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];

#if defined(DVP_USE_VRUN)
            m_pNodes[1].header.kernel = DVP_KN_VRUN_HARRIS_SCORE_7x7;
            dvp_knode_to(&m_pNodes[1], DVP_Harris_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[1], DVP_Harris_t)->harrisScore = m_images[2];
            dvp_knode_to(&m_pNodes[1], DVP_Harris_t)->k = 1310;

            m_pNodes[2].header.kernel = DVP_KN_VRUN_BLOCK_MAXIMA;      // Get Luma first
            dvp_knode_to(&m_pNodes[2], DVP_BlockMaxima_t)->input  = m_images[2];
            dvp_knode_to(&m_pNodes[2], DVP_BlockMaxima_t)->blockMaximaMAX= m_images[3];
            dvp_knode_to(&m_pNodes[2], DVP_BlockMaxima_t)->blockMaximaX= m_images[4];
            dvp_knode_to(&m_pNodes[2], DVP_BlockMaxima_t)->blockMaximaY= m_images[5];
            dvp_knode_to(&m_pNodes[2], DVP_BlockMaxima_t)->nplus1 = NPLUS1;

            m_pNodes[3].header.kernel = DVP_KN_VRUN_NMS_STEP1;
            dvp_knode_to(&m_pNodes[3], DVP_NMSStep1_t)->nmsStep1X  = m_images[4];
            dvp_knode_to(&m_pNodes[3], DVP_NMSStep1_t)->nmsStep1Y = m_images[5];
            dvp_knode_to(&m_pNodes[3], DVP_NMSStep1_t)->nmsStep1Dst = m_images[6];
            dvp_knode_to(&m_pNodes[3], DVP_NMSStep1_t)->nplus1 = NPLUS1;
#endif
            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(7))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_harrisScore");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "03_ResultMAX");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "03_ResultX");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "03_ResultY");
                ImageDebug_Init(&m_imgdbg[6], &m_images[6], m_imgdbg_path, "04_nmsStep1");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_HarrisCorner()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_VLIB) || defined(DVP_USE_VRUN)
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;

#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_MPUNONCACHED_2DTILED;
    opType = DVP_MTYPE_MPUCACHED_1DTILED;
#endif
#define NPLUS1  4
    if (m_hDVP)
    {
        if (AllocateImageStructs(7))
        {
            short new_m_width = ((m_width/NPLUS1) % (8*NPLUS1))?((m_width/NPLUS1) + (8*NPLUS1-1)) & (0xFFFF & ~(8*NPLUS1-1)) : (m_width/NPLUS1);
            short new_m_height = ((m_height/NPLUS1) % (NPLUS1))?((m_height/NPLUS1) + (NPLUS1-1)) & (0xFFFF & ~(NPLUS1-1)) : (m_height/NPLUS1);
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[3], new_m_width, new_m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[4], new_m_width, new_m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[5], new_m_width, new_m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[6], new_m_width, new_m_height, FOURCC_Y800);

            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], camType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(2) && AllocateSections(&m_graphs[0], 1))
        {
            //initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;      // Get Luma first
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input  = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];

#if defined(DVP_USE_VRUN)
            m_pNodes[1].header.kernel = DVP_KN_VRUN_HARRIS_CORNERS;
            dvp_knode_to(&m_pNodes[1], DVP_HarrisCorners_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[1], DVP_HarrisCorners_t)->output = m_images[6];
            dvp_knode_to(&m_pNodes[1], DVP_HarrisCorners_t)->tempBuf1 = m_images[2];
            dvp_knode_to(&m_pNodes[1], DVP_HarrisCorners_t)->tempBuf2 = m_images[3];
            dvp_knode_to(&m_pNodes[1], DVP_HarrisCorners_t)->tempBuf3 = m_images[4];
            dvp_knode_to(&m_pNodes[1], DVP_HarrisCorners_t)->tempBuf4 = m_images[5];
            dvp_knode_to(&m_pNodes[1], DVP_HarrisCorners_t)->k = 1310;
            dvp_knode_to(&m_pNodes[1], DVP_HarrisCorners_t)->nplus1 = NPLUS1;
#endif

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(7))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_tempHarrisScore");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "03_tempResultMAX");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "03_tempResultX");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "03_tempResultY");
                ImageDebug_Init(&m_imgdbg[6], &m_images[6], m_imgdbg_path, "04_nmsStep1");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_ORBnHarris()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_ORB)
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;

#if defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_GRALLOC_2DTILED;
#endif
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    opType  = DVP_MTYPE_MPUNONCACHED_1DTILED;
#endif
#define NPLUS1  4
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
    if (m_hDVP)
    {
        if (AllocateImageStructs(10))
        {
            unsigned short new_m_width = ((m_width/NPLUS1) % (8*NPLUS1))?((m_width/NPLUS1) + (8*NPLUS1-1)) & (0xFFFF & ~(8*NPLUS1-1)) : (m_width/NPLUS1);
            unsigned short new_m_height = ((m_height/NPLUS1) % (NPLUS1))?((m_height/NPLUS1) + (NPLUS1-1)) & (0xFFFF & ~(NPLUS1-1)) : (m_height/NPLUS1);
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_NV12);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[3], m_width+1, m_height+1, FOURCC_RGBA);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_YU24);

            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_Y16);//Harris score
            DVP_Image_Init(&m_images[6], MAX(new_m_width, m_width), MAX(new_m_height, m_height), FOURCC_Y16);//max
            DVP_Image_Init(&m_images[7], MAX(new_m_width, m_width), MAX(new_m_height, m_height), FOURCC_Y16);//X
            DVP_Image_Init(&m_images[8], MAX(new_m_width, m_width), MAX(new_m_height, m_height), FOURCC_Y16);//Y
            DVP_Image_Init(&m_images[9], MAX(new_m_width, m_width), MAX(new_m_height, m_height), FOURCC_Y800);//NMS

            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], camType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[6], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[7], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[8], opType)  ||
                !DVP_Image_Alloc(m_hDVP, &m_images[9], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateBufferStructs(3))
            {
              DVP_Buffer_Init(&m_buffers[0], sizeof(DVP_Orb_data), CORB_MAX_FEATURES);
              DVP_Buffer_Init(&m_buffers[1], sizeof(DVP_S08), CORB_NUM_PATTERNS*4);
              DVP_Buffer_Init(&m_buffers[2], 1, 192*m_width);
                if (!DVP_Buffer_Alloc(m_hDVP, &m_buffers[0], opType) ||
                    !DVP_Buffer_Alloc(m_hDVP, &m_buffers[1], opType) ||
                    !DVP_Buffer_Alloc(m_hDVP, &m_buffers[2], opType))
                    return STATUS_NOT_ENOUGH_MEMORY;
            }
            else
                return STATUS_NOT_ENOUGH_MEMORY;
#if 0
        if (AllocateNodes(3) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_VRUN_HARRIS_CORNERS;
            dvp_knode_to(&m_pNodes[0], DVP_HarrisCorners_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[0], DVP_HarrisCorners_t)->output = m_images[9];
            dvp_knode_to(&m_pNodes[0], DVP_HarrisCorners_t)->tempBuf1 = m_images[5];
            dvp_knode_to(&m_pNodes[0], DVP_HarrisCorners_t)->tempBuf2 = m_images[6];
            dvp_knode_to(&m_pNodes[0], DVP_HarrisCorners_t)->tempBuf3 = m_images[7];
            dvp_knode_to(&m_pNodes[0], DVP_HarrisCorners_t)->tempBuf4 = m_images[8];
            dvp_knode_to(&m_pNodes[0], DVP_HarrisCorners_t)->k = 1311;
            dvp_knode_to(&m_pNodes[0], DVP_HarrisCorners_t)->nplus1 = NPLUS1;
#else
        if (AllocateNodes(4) && AllocateGraphs(1) && AllocateSections(&m_graphs[0], 1))
        {
            m_pNodes[0].header.kernel = DVP_KN_CANNY_2D_GRADIENT;
            dvp_knode_to(&m_pNodes[0], DVP_Canny2dGradient_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[0], DVP_Canny2dGradient_t)->outGradX = m_images[7];
            dvp_knode_to(&m_pNodes[0], DVP_Canny2dGradient_t)->outGradY = m_images[8];
            dvp_knode_to(&m_pNodes[0], DVP_Canny2dGradient_t)->outMag = m_images[6];
            m_pNodes[0].header.affinity = DVP_CORE_DSP;

            m_pNodes[1].header.kernel = DVP_KN_VLIB_HARRIS_SCORE_7x7;
            dvp_knode_to(&m_pNodes[1], DVP_HarrisDSP_t)->inGradX = m_images[7];
            dvp_knode_to(&m_pNodes[1], DVP_HarrisDSP_t)->inGradY = m_images[8];
            dvp_knode_to(&m_pNodes[1], DVP_HarrisDSP_t)->scratch = m_buffers[2];
            dvp_knode_to(&m_pNodes[1], DVP_HarrisDSP_t)->harrisScore = m_images[5];
            dvp_knode_to(&m_pNodes[1], DVP_HarrisDSP_t)->k = 1311;
            m_pNodes[1].header.affinity = DVP_CORE_DSP;
#endif //Switch harris to simcop
            //Zero intialize the first row and column of m_images[3]
            DVP_U32 i;
            memset(m_images[3].pData[0], 0, m_images[3].width);
            for(i=0;i<m_images[3].height;i++)
                *(m_images[3].pData[0]+ i*m_images[3].y_stride) = 0;

            m_pNodes[m_numNodes-2].header.kernel = DVP_KN_INTEGRAL_IMAGE_8;
            dvp_knode_to(&m_pNodes[m_numNodes-2], DVP_Transform_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[m_numNodes-2], DVP_Transform_t)->output = m_images[3];
            dvp_knode_to(&m_pNodes[m_numNodes-2], DVP_Transform_t)->output.pData[0] = m_images[3].pBuffer[0] + m_images[3].y_stride*1 + m_images[3].x_stride*1;
            dvp_knode_to(&m_pNodes[m_numNodes-2], DVP_Transform_t)->output.y_stride = m_images[3].y_stride-1;
            dvp_knode_to(&m_pNodes[m_numNodes-2], DVP_Transform_t)->output.width = m_images[3].width-1;
            dvp_knode_to(&m_pNodes[m_numNodes-2], DVP_Transform_t)->output.height = m_images[3].height-1;
            m_pNodes[m_numNodes-2].header.affinity = DVP_CORE_DSP;
            if(dvp_knode_to(&m_pNodes[m_numNodes-2], DVP_Transform_t)->input.width> 2049 ||dvp_knode_to(&m_pNodes[m_numNodes-2], DVP_Transform_t)->input.height> 819 )
               m_pNodes[m_numNodes-2].header.affinity = DVP_CORE_CPU;

            //initialize the data in the nodes...
            m_pNodes[m_numNodes-1].header.kernel = DVP_KN_ORB;
            memcpy(m_buffers[1].pData, orbPattern31, sizeof(DVP_S08)*CORB_NUM_PATTERNS*4);
            dvp_knode_to(&m_pNodes[m_numNodes-1], DVP_Orb_t)->inputImage = m_images[1];
            dvp_knode_to(&m_pNodes[m_numNodes-1], DVP_Orb_t)->filtImage  = m_images[1];
            dvp_knode_to(&m_pNodes[m_numNodes-1], DVP_Orb_t)->harrisImage = m_images[5];
            dvp_knode_to(&m_pNodes[m_numNodes-1], DVP_Orb_t)->integralImage = m_images[3];
            dvp_knode_to(&m_pNodes[m_numNodes-1], DVP_Orb_t)->orbOutput.nfeatures = 2000;//numFeatures
            dvp_knode_to(&m_pNodes[m_numNodes-1], DVP_Orb_t)->orbOutput.orbData = m_buffers[0];
            dvp_knode_to(&m_pNodes[m_numNodes-1], DVP_Orb_t)->orbOutput.outImage = m_images[4];
            dvp_knode_to(&m_pNodes[m_numNodes-1], DVP_Orb_t)->orbPattern31 = m_buffers[1];

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(3))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "00_lumaInput");
                ImageDebug_Init(&m_imgdbg[2], &m_images[4], m_imgdbg_path, "02_orbOut");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}


status_e TestVisionEngine::Test_Boundary()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_IMGLIB)
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_MPUNONCACHED_2DTILED;
#endif

    if (m_hDVP)
    {
        if (AllocateImageStructs(6))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_Y16);
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], camType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType) || // DEFAULT is 1DTILED when using TILER
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateBufferStructs(4))
         {
             DVP_Buffer_Init(&m_buffers[0], 4, m_width*m_height);  // boundary8 outCoord (32bits)
             DVP_Buffer_Init(&m_buffers[1], 4, m_width*m_height);  // boundary8 outGray  (32bits)
             DVP_Buffer_Init(&m_buffers[2], 4, m_width*m_height);  // boundary16 outCoord (32bits)
             DVP_Buffer_Init(&m_buffers[3], 2, m_width*m_height);  // boundary16 outGray  (16bits)
             if (!DVP_Buffer_Alloc(m_hDVP, &m_buffers[0], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[1], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[2], DVP_MTYPE_DEFAULT) ||
                 !DVP_Buffer_Alloc(m_hDVP, &m_buffers[3], DVP_MTYPE_DEFAULT))
                 return STATUS_NOT_ENOUGH_MEMORY;
         }
         else
             return STATUS_NOT_ENOUGH_MEMORY;

        if (AllocateNodes(7) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...
            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;      // Get Luma first
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];
            //m_pNodes[0].header.affinity = DVP_CORE_CPU;

            m_pNodes[1].header.kernel = DVP_KN_THRESHOLD;         // Then threshold for binary image functions
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[1];       // Using Luma from Luma Extract function
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[2];      // Thresholded output
            //m_pNodes[1].header.affinity = DVP_CORE_CPU;

            m_pNodes[2].header.kernel = DVP_KN_XSTRIDE_SHIFT;
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->input = m_images[2];       // Use 8bit luma
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->output = m_images[3];      // Output 16bit luma
            //m_pNodes[2].header.affinity = DVP_CORE_CPU;

            m_pNodes[3].header.kernel = DVP_KN_IMG_BOUNDARY_8;
            dvp_knode_to(&m_pNodes[3], DVP_Boundary_t)->input = m_images[2];     // Use 8bit luma
            dvp_knode_to(&m_pNodes[3], DVP_Boundary_t)->outCoord = m_buffers[0];
            dvp_knode_to(&m_pNodes[3], DVP_Boundary_t)->outGray = m_buffers[1];
            //m_pNodes[3].header.affinity = DVP_CORE_CPU;

            m_pNodes[4].header.kernel = DVP_KN_IMG_BOUNDARY_16s;
            dvp_knode_to(&m_pNodes[4], DVP_Boundary_t)->input = m_images[3];     // Use 16bit luma
            dvp_knode_to(&m_pNodes[4], DVP_Boundary_t)->outCoord = m_buffers[2];
            dvp_knode_to(&m_pNodes[4], DVP_Boundary_t)->outGray = m_buffers[3];
            //m_pNodes[4].header.affinity = DVP_CORE_CPU;

            m_pNodes[5].header.kernel = DVP_KN_IMG_PERIMETER_8;
            dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->input = m_images[2];           // Use 8bit luma
            dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->output = m_images[4];
            //m_pNodes[5].header.affinity = DVP_CORE_CPU;

            m_pNodes[6].header.kernel = DVP_KN_IMG_PERIMETER_16;
            dvp_knode_to(&m_pNodes[6], DVP_Transform_t)->input = m_images[3];           // Use 16bit luma
            dvp_knode_to(&m_pNodes[6], DVP_Transform_t)->output = m_images[5];
            //m_pNodes[6].header.affinity = DVP_CORE_CPU;

            memset(m_buffers[0].pData, 0, m_buffers[0].numBytes);
            memset(m_buffers[1].pData, 0, m_buffers[1].numBytes);
            memset(m_buffers[2].pData, 0, m_buffers[2].numBytes);
            memset(m_buffers[3].pData, 0, m_buffers[3].numBytes);

            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }
        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(10))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_input");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_bitimgThr");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "03_bitimgThr16");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "04_bitimgPerim8");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "05_bitimgPerim16");
                BufferDebug_Init(&m_imgdbg[6], &m_buffers[0], m_imgdbg_path, "06_bound8_coord", ".bw");
                BufferDebug_Init(&m_imgdbg[7], &m_buffers[1], m_imgdbg_path, "07_bound8_gray", ".bw");
                BufferDebug_Init(&m_imgdbg[8], &m_buffers[2], m_imgdbg_path, "08_bound16_coord", ".bw");
                BufferDebug_Init(&m_imgdbg[9], &m_buffers[3], m_imgdbg_path, "09_bound16_gray", ".bw");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
        status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}

status_e TestVisionEngine::Test_RVM()
{
    status_e status = STATUS_SUCCESS;
#if defined(DVP_USE_RVM)
    char calibration_file[MAX_PATH];
    char lut_file[MAX_PATH];
    char system_command[MAX_PATH];
    int blockSize = 32;

    /**< The name of the lut binary file */
    sprintf(calibration_file, "raw/rvm_calib_%ux%u_to_%ux%u.txt", m_width, m_height, m_width, m_height);
    sprintf(lut_file, "raw/rvm_lut_%ux%u_to_%ux%u_BL%d.bin", m_width, m_height, m_width, m_height, blockSize);

    FILE *lut, *calib;                         /**< filehandles */
    DVP_MemType_e camType = DVP_MTYPE_DEFAULT;
    DVP_MemType_e opType = DVP_MTYPE_DEFAULT;
#if defined(DVP_USE_TILER) || defined(DVP_USE_ION) || defined(DVP_USE_BO)
    camType = DVP_MTYPE_MPUNONCACHED_2DTILED;
#endif

    if (m_hDVP)
    {
        if (AllocateImageStructs(6))
        {
            DVP_Image_Init(&m_images[0], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[1], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[2], m_width, m_height, FOURCC_Y16);
            DVP_Image_Init(&m_images[3], m_width, m_height, FOURCC_UYVY);
            DVP_Image_Init(&m_images[4], m_width, m_height, FOURCC_Y800);
            DVP_Image_Init(&m_images[5], m_width, m_height, FOURCC_Y16);
            if (!DVP_Image_Alloc(m_hDVP, &m_images[0], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[1], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[2], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[3], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[4], opType) ||
                !DVP_Image_Alloc(m_hDVP, &m_images[5], opType))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        // Check if LUT file already exists: if it does, then open it and read the LUT,
        //                                   if it doesn't, then generate it using the calibration file.
        lut = fopen(lut_file, "rb");
        if (lut == NULL) {
            calib = fopen(calibration_file, "r");
            if (calib == NULL) {
                DVP_PRINT(DVP_ZONE_ERROR, "Error opening file %s for reading\n",calibration_file);
                return STATUS_NO_RESOURCES;
            }
            fclose(calib);
            DVP_PRINT(DVP_ZONE_ENGINE, "Generating file %s for rvm LUT input!\n",lut_file);
            sprintf(system_command, "rvm_generate_lut %d %d %d %d %d raw/rvm_calib_%dx%d_to_%dx%d.txt raw/rvm_lut_%dx%d_to_%dx%d_BL%d.bin -ORDERED_OUTPUT",
                m_width, m_height, m_width, m_height, blockSize,
                m_width, m_height, m_width, m_height,
                m_width, m_height, m_width, m_height, blockSize);
            system(system_command);
            lut = fopen(lut_file, "rb");
            if (lut == NULL) {
                DVP_PRINT(DVP_ZONE_ERROR, "Error opening file %s for reading\n",lut_file);
                return STATUS_NO_RESOURCES;
            }
        }
        DVP_PRINT(DVP_ZONE_ENGINE, "Opening file %s for rvm LUT input!\n",lut_file);

        fseek( lut, 0, SEEK_END );
        int fileSize = ftell( lut );
        rewind( lut );

        if (AllocateBufferStructs(1))
        {
            DVP_Buffer_Init(&m_buffers[0], 1, fileSize);
            if (!DVP_Buffer_Alloc(m_hDVP, &m_buffers[0], DVP_MTYPE_DEFAULT))
                return STATUS_NOT_ENOUGH_MEMORY;
        }
        else
            return STATUS_NOT_ENOUGH_MEMORY;

        fread( m_buffers[0].pData, 1, fileSize, lut );
        fclose( lut );

        if (AllocateNodes(4) && AllocateSections(&m_graphs[0], 1))
        {
            // initialize the data in the nodes...

            m_pNodes[0].header.kernel = DVP_KN_XYXY_TO_Y800;
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = m_images[0];   //422
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->output = m_images[1];  //luma8

            m_pNodes[1].header.kernel = DVP_KN_XSTRIDE_SHIFT;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = m_images[1];   //luma8
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->output = m_images[2];  //luma16

            m_pNodes[2].header.kernel = DVP_KN_RVM; //422 to 422
            dvp_knode_to(&m_pNodes[2], DVP_Rvm_t)->input = m_images[0];
            dvp_knode_to(&m_pNodes[2], DVP_Rvm_t)->output = m_images[3];
            dvp_knode_to(&m_pNodes[2], DVP_Rvm_t)->lut = m_buffers[0];

            m_pNodes[3].header.kernel = DVP_KN_RVM; //luma8 to luma8
            dvp_knode_to(&m_pNodes[3], DVP_Rvm_t)->input = m_images[1];
            dvp_knode_to(&m_pNodes[3], DVP_Rvm_t)->output = m_images[4];
            dvp_knode_to(&m_pNodes[3], DVP_Rvm_t)->lut = m_buffers[0];
#if 0
            m_pNodes[4].header.kernel = DVP_KN_RVM; //luma16 to luma16
            dvp_knode_to(&m_pNodes[4], DVP_Rvm_t)->input = m_images[2];
            dvp_knode_to(&m_pNodes[4], DVP_Rvm_t)->output = m_images[5];
            dvp_knode_to(&m_pNodes[4], DVP_Rvm_t)->lut = m_buffers[0];
#endif
            // put all the nodes in the section.
            m_graphs[0].sections[0].pNodes = &m_pNodes[0];
            m_graphs[0].sections[0].numNodes = m_numNodes;
            m_graphs[0].order[0] = 0;
        }

        if(m_testcore != DVP_CORE_MIN) {
            for(uint32_t testnode=0; testnode<m_numNodes; testnode++) {
                m_pNodes[testnode].header.affinity = m_testcore;
            }
        }

        status = CameraInit(this, m_images[0].color);
        if (status == STATUS_SUCCESS)
        {
            if (m_imgdbg_enabled && AllocateImageDebug(6))
            {
                ImageDebug_Init(&m_imgdbg[0], &m_images[0], m_imgdbg_path, "00_yuvInput");
                ImageDebug_Init(&m_imgdbg[1], &m_images[1], m_imgdbg_path, "01_luma8Input");
                ImageDebug_Init(&m_imgdbg[2], &m_images[2], m_imgdbg_path, "02_luma16Input");
                ImageDebug_Init(&m_imgdbg[3], &m_images[3], m_imgdbg_path, "03_yuvOutput");
                ImageDebug_Init(&m_imgdbg[4], &m_images[4], m_imgdbg_path, "04_luma8Output");
                ImageDebug_Init(&m_imgdbg[5], &m_images[5], m_imgdbg_path, "05_luma16Output");
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            // clear the performance
            DVP_Perf_Clear(&m_graphs[0].totalperf);
            DVP_PerformanceClear(m_hDVP, m_pNodes, m_numNodes);
            DVP_PRINT(DVP_ZONE_ENGINE, "About to process %u frames!\n",m_numFrames);
        }
    }
    else
        status = STATUS_NO_RESOURCES;
#else
    status = STATUS_NOT_IMPLEMENTED;
#endif
    return status;
}


status_e TestVisionEngine::GraphUpdate(VisionCamFrame *cameraFrame)
{
    DVP_U32 i = 0;
    if (m_running && (cameraFrame->mFrameSource == VCAM_PORT_ALL ||
                      cameraFrame->mFrameSource == VCAM_PORT_PREVIEW))
    {
        DVP_Image_t *pImage = (DVP_Image_t *)cameraFrame->mFrameBuff;
        // update the graph with the image that was given to us
        if (m_graphtype == TI_GRAPH_TYPE_TEST4)
        {
            // this is an implicit memcpy
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = *pImage;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = *pImage;
            dvp_knode_to(&m_pNodes[2], DVP_Transform_t)->input = *pImage;
            dvp_knode_to(&m_pNodes[5], DVP_Transform_t)->input = *pImage;
            DVP_PRINT(DVP_ZONE_ENGINE, "Updating Images in Graph with Returned Buffer from Camera %p\n", pImage);
            DVP_PrintImage(DVP_ZONE_ENGINE, pImage);
            DVP_PRINT(DVP_ZONE_ENGINE, "Dumping all images:\n");
            for (i = 0; i < m_numImages; i++)
                DVP_PrintImage(DVP_ZONE_ENGINE, &m_images[i]);
        }
        else if (m_graphtype == TI_GRAPH_TYPE_TEST9)
        {
            DVP_PRINT(DVP_ZONE_ENGINE, "Updating Image in TEST9 Graph!\n");
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = *pImage;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = *pImage;
            //m_pNodes[4], DVP_Transform_t)->input = *pImage;
        }
        else if (m_graphtype == TI_GRAPH_TYPE_TEST10)
        {
            DVP_PRINT(DVP_ZONE_ENGINE, "Updating Image in TEST10 Graph!\n");
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = *pImage;
            dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = *pImage;
        }
        else if (m_graphtype == TI_GRAPH_TYPE_TEST11)
        {
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = *pImage;
            dvp_knode_to(&m_pNodes[3], DVP_Transform_t)->input = *pImage;
            dvp_knode_to(&m_pNodes[4], DVP_Transform_t)->input = *pImage;
        }
        else if (m_graphtype == TI_GRAPH_TYPE_BOUNDARY)
        {
            memset(m_buffers[0].pData, 0, m_buffers[0].numBytes);
            memset(m_buffers[1].pData, 0, m_buffers[1].numBytes);
            memset(m_buffers[2].pData, 0, m_buffers[2].numBytes);
            memset(m_buffers[3].pData, 0, m_buffers[3].numBytes);
        }
#if defined(DVP_USE_VLIB) && defined(DVP_USE_YUV)
        else if (m_graphtype == TI_GRAPH_TYPE_DISPARITY)
        {
            // construct the left and right images from the camera input
            DVP_Image_t left = *pImage;
            DVP_Image_t right = *pImage;

            // we're assuming LEFT/RIGHT NV12
            left.width >>= 1;
            right.width >>= 1;

            // y stride should be the same
            right.pData[0] = left.pData[0] + (left.width*left.x_stride);
            right.pData[1] = left.pData[1] + ((left.width/2)*left.x_stride*2);

            dvp_knode_to(&m_pNodes[0], DVP_Disparity_t)->left = left;
            dvp_knode_to(&m_pNodes[0], DVP_Disparity_t)->right = right;
        }
#endif
#if defined(DVP_USE_TISMO)
        else if (m_graphtype == TI_GRAPH_TYPE_TISMO)
        {
            // construct the left and right images from the camera input
            DVP_Image_t left = *pImage;
            DVP_Image_t right = *pImage;
            DVP_Image_t disp = *pImage;

            left.width = dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->output.width;
            left.height = dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->output.height;
            right.width = dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->output.width;
            right.height = dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->output.height;
            disp.width = dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->output.width;
            disp.height = dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->output.height;

            // y stride should be the same
#if TOPBOTTOM
            right.pData[0] = left.pBuffer[0] + left.height*left.y_stride;
#if DISPLAY_DISPARITY
            disp.pData[0]  = left.pBuffer[0] + left.width*left.x_stride;
            disp.pData[1]  = left.pBuffer[1] + left.width*left.x_stride;
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->output = disp;
#endif
#else
            right.pData[0] = left.pBuffer[0] + left.width*left.x_stride;
#endif
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->left = left;
            //WORKAROUND
            //DVP_Image_Copy(&m_images[m_dispMax+2], &right);
            dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->right = right;

            if (m_imgdbg_enabled){
                memcpy(&m_dbgImage, pImage, sizeof(DVP_Image_t));//Copy image structure

                m_imgdbg[0].pImg = &m_dbgImage;
#if TOPBOTTOM
#if DISPLAY_DISPARITY
                m_imgdbg[0].pImg->width *= 2;
#endif
                m_imgdbg[0].pImg->height = m_height;
#else
                m_imgdbg[0].pImg->width = m_width;
#endif
                m_imgdbg[3].pImg = &dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->left;
                m_imgdbg[4].pImg = &dvp_knode_to(&m_pNodes[0], DVP_Tismo_t)->right;
            }
        }
#endif // defined(DVP_USE_TISMO)
#if defined(DVP_USE_TISMOV02)
        else if (m_graphtype == TI_GRAPH_TYPE_TISMOV02)
        {
            // construct the left and right images from the camera input
            DVP_Image_t left = *pImage;
            DVP_Image_t right = *pImage;
            DVP_Image_t disp = *pImage;

            left.width = dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_lumaU8.width;
            left.height = dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_lumaU8.height;
            right.width = dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_lumaU8.width;
            right.height = dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_lumaU8.height;
            disp.width = dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_lumaU8.width;
            disp.height = dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_lumaU8.height;

            // y stride should be the same
#if TOPBOTTOM
            right.pData[0] = left.pBuffer[0] + left.height*left.y_stride;
#if DISPLAY_DISPARITY
            disp.pData[0]  = left.pBuffer[0] + left.width*left.x_stride;
            disp.pData[1]  = left.pBuffer[1] + left.width*left.x_stride;
            dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->out_falseColor_NV12 = disp;
#endif
#else
            right.pData[0] = left.pBuffer[0] + left.width*left.x_stride;
#endif
            //dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->in_left = left;
            //WORKAROUND
            DVP_Image_Copy(&m_images[m_dispMax+7], &left);
            DVP_Image_Copy(&m_images[m_dispMax+8], &right);
            //dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->in_right = right;

            //dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = left;
            //dvp_knode_to(&m_pNodes[1], DVP_Transform_t)->input = right;

            if (m_imgdbg_enabled){
                memcpy(&m_dbgImage, pImage, sizeof(DVP_Image_t));//Copy image structure

                m_imgdbg[0].pImg = &m_dbgImage;
#if TOPBOTTOM
#if DISPLAY_DISPARITY
                m_imgdbg[0].pImg->width *= 2;
#endif
                m_imgdbg[0].pImg->height = m_height;
#else
                m_imgdbg[0].pImg->width = m_width;
#endif
                m_imgdbg[3].pImg = &dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->in_left;
                m_imgdbg[4].pImg = &dvp_knode_to(&m_pNodes[2], DVP_TismoV02_t)->in_right;
            }
        }
#endif // defined(DVP_USE_TISMOV02)
#if defined(DVP_USE_ORB)
        else if(m_graphtype == TI_GRAPH_TYPE_ORBHARRIS)
        {
            DVP_Image_Copy(&m_images[1], pImage);
            memcpy(dvp_knode_to(&m_pNodes[m_numNodes-1], DVP_Orb_t)->orbOutput.outImage.pData[0], m_images[1].pData[0] , m_width*m_height*sizeof(DVP_U08));
            //dvp_knode_to(&m_pNodes[0], DVP_HarrisCorners_t)->input = *pImage;
            //dvp_knode_to(&m_pNodes[m_numNodes-2], DVP_Transform_t)->input = *pImage;
            //dvp_knode_to(&m_pNodes[m_numNodes-1], DVP_Orb_t)->inputImage = *pImage;
        }
#endif
        else
            dvp_knode_to(&m_pNodes[0], DVP_Transform_t)->input = *pImage;

        // find the camera buffer and point the display index to it.
        for (i = m_camMin; i < m_camMax; i++)
            if (pImage == &m_images[i])
                m_dispIdx = i;

    }
    return STATUS_SUCCESS;
}


void TestVisionEngine::GraphSectionComplete(DVP_KernelGraph_t *graph, uint32_t sectionIndex, uint32_t numNodesExecuted)
{
    graph = graph; // warnings
    sectionIndex = sectionIndex; // warnings
    numNodesExecuted = numNodesExecuted; // warnings
    DVP_PRINT(DVP_ZONE_ENGINE, "Graph %p Section %u Completed %u nodes!\n", graph, sectionIndex, numNodesExecuted);
}
