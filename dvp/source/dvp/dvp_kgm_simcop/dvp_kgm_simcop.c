/**
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

#include <sosal/sosal.h>

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>
#include <dvp_kgm.h>

#if defined(DVP_USE_VRUN)
#include <vrun/dvp_kl_vrun.h>
#endif

#if defined(DVP_USE_DEI)
#include <dei/dvp_kl_dei.h>
#endif

#define KGM_TAG "DVP KGM SIMCOP"

static dvp_image_shift_t imx_shift3 = {
    -1, -1, 0, 2, 2, 0,
};

static dvp_image_shift_t imx_shift5 = {
    -2, -2, 0, 4, 4, 0,
};

static dvp_image_shift_t imx_shift7 = {
    -3, -3, 0, 6, 6, 0,
};

static dvp_image_shift_t imx_shift8 = {
    -4, -4, 0, 7, 7, 0,
};

static dvp_image_shift_t imx_shift16 = {
    -8, -8, 0, 15, 15, 0,
};

static dvp_image_shift_t imx_nonmax_shift3 = {
    -1, 1, 2, 2, 0, 0,
};

static dvp_image_shift_t imx_nonmax_shift5 = {
    -2, 2, 4, 4, 0, 0,
};

static dvp_image_shift_t imx_nonmax_shift7 = {
    -3, 3, 6, 6, 0, 0,
};

static void dvp_vrun_conv_mxn_shift(DVP_KernelNode_t *node, dvp_image_shift_t *shift)
{
    if (node && shift)
    {
        DVP_ImageConvolution_t *pC = dvp_knode_to(node, DVP_ImageConvolution_t);
        shift->centerShiftHorz = -(DVP_S32)pC->mask.width/2;
        shift->centerShiftVert = -(DVP_S32)pC->mask.height/2;
        shift->rightBorder = pC->mask.width - 1;
        shift->bottomBorder = pC->mask.height - 1;
    }
}

static DVP_CoreFunction_t remote_kernels[] = {
#ifdef DVP_USE_IPC
    {"SIMCOP No operation",    DVP_KN_NOOP,                         0, NULL, NULL},
    {"SIMCOP Echo",            DVP_KN_COPY,                         0, NULL, NULL},
#ifdef DVP_USE_VRUN
    // name                    kernel                               load  shift   shift_func
    {"SIMCOP Dilate Cross",    DVP_KN_DILATE_CROSS,                 0, &imx_shift3, NULL},
    {"SIMCOP Dilate Mask",     DVP_KN_DILATE_MASK,                  0, &imx_shift3, NULL},
    {"SIMCOP Dilate Square",   DVP_KN_DILATE_SQUARE,                0, &imx_shift3, NULL},
    {"SIMCOP Erode Cross",     DVP_KN_ERODE_CROSS,                  0, &imx_shift3, NULL},
    {"SIMCOP Erode Mask",      DVP_KN_ERODE_MASK,                   0, &imx_shift3, NULL},
    {"SIMCOP Erode Cross",     DVP_KN_ERODE_SQUARE,                 0, &imx_shift3, NULL},

    {"SIMCOP Dilate Cross",    DVP_KN_VRUN_DILATE_CROSS,            0, &imx_shift3, NULL},
    {"SIMCOP Dilate Mask",     DVP_KN_VRUN_DILATE_MASK,             0, &imx_shift3, NULL},
    {"SIMCOP Dilate Square",   DVP_KN_VRUN_DILATE_SQUARE,           0, &imx_shift3, NULL},
    {"SIMCOP Erode Cross",     DVP_KN_VRUN_ERODE_CROSS,             0, &imx_shift3, NULL},
    {"SIMCOP Erode Mask",      DVP_KN_VRUN_ERODE_MASK,              0, &imx_shift3, NULL},
    {"SIMCOP Erode Cross",     DVP_KN_VRUN_ERODE_SQUARE,            0, &imx_shift3, NULL},

    {"SIMCOP IIR Horz",        DVP_KN_IIR_HORZ,                     0, NULL, NULL},
    {"SIMCOP IIR Vert",        DVP_KN_IIR_VERT,                     0, NULL, NULL},

    {"SIMCOP IIR Horz",        DVP_KN_VRUN_IIR_HORZ,                0, NULL, NULL},
    {"SIMCOP IIR Vert",        DVP_KN_VRUN_IIR_VERT,                0, NULL, NULL},

    {"SIMCOP NonMax Sup 3x3",  DVP_KN_NONMAXSUPPRESS_3x3_S16,       0, &imx_nonmax_shift3, NULL},
    {"SIMCOP NonMax Sup 5x5",  DVP_KN_NONMAXSUPPRESS_5x5_S16,       0, &imx_nonmax_shift5, NULL},
    {"SIMCOP NonMax Sup 7x7",  DVP_KN_NONMAXSUPPRESS_7x7_S16,       0, &imx_nonmax_shift7, NULL},

    {"SIMCOP NonMax Sup 3x3",  DVP_KN_VRUN_NONMAXSUPPRESS_3x3_S16,  0, &imx_nonmax_shift3, NULL},
    {"SIMCOP NonMax Sup 5x5",  DVP_KN_VRUN_NONMAXSUPPRESS_5x5_S16,  0, &imx_nonmax_shift5, NULL},
    {"SIMCOP NonMax Sup 7x7",  DVP_KN_VRUN_NONMAXSUPPRESS_7x7_S16,  0, &imx_nonmax_shift7, NULL},

    {"SIMCOP Img Conv 3x3",    DVP_KN_CONV_3x3,                     0, &imx_shift3, NULL},
    {"SIMCOP Img Conv 5x5",    DVP_KN_CONV_5x5,                     0, &imx_shift5, NULL},
    {"SIMCOP Img Conv 7x7",    DVP_KN_CONV_7x7,                     0, &imx_shift7, NULL},

    {"SIMCOP Img Conv 3x3",    DVP_KN_VRUN_CONV_3x3,                0, &imx_shift3, NULL},
    {"SIMCOP Img Conv 5x5",    DVP_KN_VRUN_CONV_5x5,                0, &imx_shift5, NULL},
    {"SIMCOP Img Conv 7x7",    DVP_KN_VRUN_CONV_7x7,                0, &imx_shift7, NULL},
    {"SIMCOP Img Conv MxN",    DVP_KN_VRUN_CONV_MxN,                0, NULL, dvp_vrun_conv_mxn_shift},

    {"SIMCOP Img Thr gt2max8",   DVP_KN_THR_GT2MAX_8,               0, NULL, NULL},
    {"SIMCOP Img Thr gt2max16",  DVP_KN_THR_GT2MAX_16,              0, NULL, NULL},
    {"SIMCOP Img Thr gt2thr8",   DVP_KN_THR_GT2THR_8,               0, NULL, NULL},
    {"SIMCOP Img Thr gt2thr16",  DVP_KN_THR_GT2THR_16,              0, NULL, NULL},
    {"SIMCOP Img Thr le2min8",   DVP_KN_THR_LE2MIN_8,               0, NULL, NULL},
    {"SIMCOP Img Thr le2min16",  DVP_KN_THR_LE2MIN_16,              0, NULL, NULL},
    {"SIMCOP Img Thr le2thr8",   DVP_KN_THR_LE2THR_8,               0, NULL, NULL},
    {"SIMCOP Img Thr le2thr16",  DVP_KN_THR_LE2THR_16,              0, NULL, NULL},

    {"SIMCOP Img Thr gt2max8",   DVP_KN_VRUN_THR_GT2MAX_8,          0, NULL, NULL},
    {"SIMCOP Img Thr gt2max16",  DVP_KN_VRUN_THR_GT2MAX_16,         0, NULL, NULL},
    {"SIMCOP Img Thr gt2thr8",   DVP_KN_VRUN_THR_GT2THR_8,          0, NULL, NULL},
    {"SIMCOP Img Thr gt2thr16",  DVP_KN_VRUN_THR_GT2THR_16,         0, NULL, NULL},
    {"SIMCOP Img Thr le2min8",   DVP_KN_VRUN_THR_LE2MIN_8,          0, NULL, NULL},
    {"SIMCOP Img Thr le2min16",  DVP_KN_VRUN_THR_LE2MIN_16,         0, NULL, NULL},
    {"SIMCOP Img Thr le2thr8",   DVP_KN_VRUN_THR_LE2THR_8,          0, NULL, NULL},
    {"SIMCOP Img Thr le2thr16",  DVP_KN_VRUN_THR_LE2THR_16,         0, NULL, NULL},

    {"SIMCOP Img Sobel 3x3 8s",  DVP_KN_SOBEL_3x3_8s,               0, &imx_shift3, NULL},
    {"SIMCOP Img Sobel 3x3 8",   DVP_KN_SOBEL_3x3_8,                0, &imx_shift3, NULL},
    {"SIMCOP Img Sobel 3x3 16s", DVP_KN_SOBEL_3x3_16s,              0, &imx_shift3, NULL},
    {"SIMCOP Img Sobel 3x3 16",  DVP_KN_SOBEL_3x3_16,               0, &imx_shift3, NULL},
    {"SIMCOP Img Sobel 5x5 8s",  DVP_KN_SOBEL_5x5_8s,               0, &imx_shift5, NULL},
    {"SIMCOP Img Sobel 5x5 8",   DVP_KN_SOBEL_5x5_8,                0, &imx_shift5, NULL},
    {"SIMCOP Img Sobel 5x5 16s", DVP_KN_SOBEL_5x5_16s,              0, &imx_shift5, NULL},
    {"SIMCOP Img Sobel 5x5 16",  DVP_KN_SOBEL_5x5_16,               0, &imx_shift5, NULL},
    {"SIMCOP Img Sobel 7x7 8s",  DVP_KN_SOBEL_7x7_8s,               0, &imx_shift7, NULL},
    {"SIMCOP Img Sobel 7x7 8",   DVP_KN_SOBEL_7x7_8,                0, &imx_shift7, NULL},
    {"SIMCOP Img Sobel 7x7 16s", DVP_KN_SOBEL_7x7_16s,              0, &imx_shift7, NULL},
    {"SIMCOP Img Sobel 7x7 16",  DVP_KN_SOBEL_7x7_16,               0, &imx_shift7, NULL},

    {"SIMCOP Img Sobel 3x3 8s",  DVP_KN_VRUN_SOBEL_3x3_8s,          0, &imx_shift3, NULL},
    {"SIMCOP Img Sobel 3x3 8",   DVP_KN_VRUN_SOBEL_3x3_8,           0, &imx_shift3, NULL},
    {"SIMCOP Img Sobel 3x3 16s", DVP_KN_VRUN_SOBEL_3x3_16s,         0, &imx_shift3, NULL},
    {"SIMCOP Img Sobel 3x3 16",  DVP_KN_VRUN_SOBEL_3x3_16,          0, &imx_shift3, NULL},
    {"SIMCOP Img Sobel 5x5 8s",  DVP_KN_VRUN_SOBEL_5x5_8s,          0, &imx_shift5, NULL},
    {"SIMCOP Img Sobel 5x5 8",   DVP_KN_VRUN_SOBEL_5x5_8,           0, &imx_shift5, NULL},
    {"SIMCOP Img Sobel 5x5 16s", DVP_KN_VRUN_SOBEL_5x5_16s,         0, &imx_shift5, NULL},
    {"SIMCOP Img Sobel 5x5 16",  DVP_KN_VRUN_SOBEL_5x5_16,          0, &imx_shift5, NULL},
    {"SIMCOP Img Sobel 7x7 8s",  DVP_KN_VRUN_SOBEL_7x7_8s,          0, &imx_shift7, NULL},
    {"SIMCOP Img Sobel 7x7 8",   DVP_KN_VRUN_SOBEL_7x7_8,           0, &imx_shift7, NULL},
    {"SIMCOP Img Sobel 7x7 16s", DVP_KN_VRUN_SOBEL_7x7_16s,         0, &imx_shift7, NULL},
    {"SIMCOP Img Sobel 7x7 16",  DVP_KN_VRUN_SOBEL_7x7_16,          0, &imx_shift7, NULL},

    {"SIMCOP Img Harris Corners",DVP_KN_VRUN_HARRIS_CORNERS,        0, NULL, NULL},
    {"SIMCOP Img Harris Score",  DVP_KN_VRUN_HARRIS_SCORE_7x7,      0, NULL, NULL},
    {"SIMCOP Img Block Maxima",  DVP_KN_VRUN_BLOCK_MAXIMA,          0, NULL, NULL},
    {"SIMCOP Img NMS Step1",     DVP_KN_VRUN_NMS_STEP1,             0, NULL, NULL},

    {"SIMCOP Canny Img",       DVP_KN_CANNY_IMAGE_SMOOTHING,        0, &imx_shift7, NULL},
    {"SIMCOP Canny 2D Grad",   DVP_KN_CANNY_2D_GRADIENT,            0, &imx_shift3, NULL},
    {"SIMCOP Canny NonMax",    DVP_KN_CANNY_NONMAX_SUPPRESSION,     0, &imx_shift3, NULL},
    {"SIMCOP Canny Hyst Th",   DVP_KN_CANNY_HYST_THRESHHOLD,        0, NULL, NULL},

    {"SIMCOP Canny Img",       DVP_KN_VRUN_CANNY_IMAGE_SMOOTHING,   0, &imx_shift7, NULL},
    {"SIMCOP Canny 2D Grad",   DVP_KN_VRUN_CANNY_2D_GRADIENT,       0, &imx_shift3, NULL},
    {"SIMCOP Canny NonMax",    DVP_KN_VRUN_CANNY_NONMAX_SUPPRESSION, 0, &imx_shift3, NULL},
    {"SIMCOP Canny Hyst Th",   DVP_KN_VRUN_CANNY_HYST_THRESHHOLD,   0, NULL, NULL},

    {"SIMCOP UYVY to YUV444p", DVP_KN_UYVY_TO_YUV444p,              0, NULL, NULL},
    {"SIMCOP UYVY to RGBp",    DVP_KN_UYVY_TO_RGBp,                 0, NULL, NULL},
    {"SIMCOP UYVY to IYUV",    DVP_KN_UYVY_TO_YUV420p,              0, NULL, NULL},
    {"SIMCOP xYxY to LUMA",    DVP_KN_XYXY_TO_Y800,                 0, NULL, NULL},
    {"SIMCOP NV12 to YUV444p", DVP_KN_NV12_TO_YUV444p,              0, NULL, NULL},
    {"SIMCOP YUV444p to RGBp", DVP_KN_YUV444p_TO_RGBp,              0, NULL, NULL},

    {"SIMCOP UYVY to YUV444p", DVP_KN_VRUN_UYVY_TO_YUV444p,         0, NULL, NULL},
    {"SIMCOP UYVY to RGBp",    DVP_KN_VRUN_UYVY_TO_RGBp,            0, NULL, NULL},
    {"SIMCOP UYVY to IYUV",    DVP_KN_VRUN_UYVY_TO_YUV420p,         0, NULL, NULL},
    {"SIMCOP xYxY to LUMA",    DVP_KN_VRUN_XYXY_TO_Y800,            0, NULL, NULL},
    {"SIMCOP NV12 to YUV444p", DVP_KN_VRUN_NV12_TO_YUV444p,         0, NULL, NULL},
    {"SIMCOP YUV444p to RGBp", DVP_KN_VRUN_YUV444p_TO_RGBp,         0, NULL, NULL},

    {"SIMCOP Integral 8",      DVP_KN_INTEGRAL_IMAGE_8,             0, NULL, NULL},

    {"SIMCOP Integral 8",      DVP_KN_VRUN_INTEGRAL_IMAGE_8,        0, NULL, NULL},

    {"LDC Affine Transfm",     DVP_KN_LDC_AFFINE_TRANSFORM,         0, NULL, NULL},
    {"LDC Distortion Correct", DVP_KN_LDC_DISTORTION_CORRECTION,    0, NULL, NULL},
    {"LDC Distortion & Aff",   DVP_KN_LDC_DISTORTION_AND_AFFINE,    0, NULL, NULL},
    {"SIMCOP SAD 8x8",         DVP_KN_VRUN_SAD_8x8,                 0, &imx_shift8, NULL},
    {"SIMCOP SAD 16x16",       DVP_KN_VRUN_SAD_16x16,               0, &imx_shift16, NULL},
    {"SIMCOP SAD 3x3",         DVP_KN_VRUN_SAD_3x3,                 0, &imx_shift3, NULL},
    {"SIMCOP SAD 5x5",         DVP_KN_VRUN_SAD_5x5,                 0, &imx_shift5, NULL},
    {"SIMCOP SAD 7x7",         DVP_KN_VRUN_SAD_7x7,                 0, &imx_shift7, NULL},

    {"SIMCOP ImgPyramid8",          DVP_KN_VRUN_IMAGE_PYRAMID_8,         0, NULL, NULL},
    {"SIMCOP Gauss3x3Pyramid8",     DVP_KN_VRUN_GAUSSIAN_3x3_PYRAMID_8,  0, &imx_shift3, NULL},
    {"SIMCOP Gauss5x5Pyramid8",     DVP_KN_VRUN_GAUSSIAN_5x5_PYRAMID_8,  0, &imx_shift5, NULL},
    {"SIMCOP Gauss7x7Pyramid8",     DVP_KN_VRUN_GAUSSIAN_7x7_PYRAMID_8,  0, &imx_shift7, NULL},
    {"SIMCOP GradientH3x3Pyramid8", DVP_KN_VRUN_GRADIENT_H3x3_PYRAMID_8, 0, &imx_shift3, NULL},
    {"SIMCOP GradientH5x5Pyramid8", DVP_KN_VRUN_GRADIENT_H5x5_PYRAMID_8, 0, &imx_shift5, NULL},
    {"SIMCOP GradientH7x7Pyramid8", DVP_KN_VRUN_GRADIENT_H7x7_PYRAMID_8, 0, &imx_shift7, NULL},
    {"SIMCOP GradientV3x3Pyramid8", DVP_KN_VRUN_GRADIENT_V3x3_PYRAMID_8, 0, &imx_shift3, NULL},
    {"SIMCOP GradientV5x5Pyramid8", DVP_KN_VRUN_GRADIENT_V5x5_PYRAMID_8, 0, &imx_shift5, NULL},
    {"SIMCOP GradientV7x7Pyramid8", DVP_KN_VRUN_GRADIENT_V7x7_PYRAMID_8, 0, &imx_shift7, NULL},

#endif
#ifdef DVP_USE_DEI
    {"SIMCOP DEI INIT",        DVP_KN_DEI_DEINTERLACER_INIT,        0, NULL, NULL},
    {"SIMCOP DEI DEINIT",      DVP_KN_DEI_DEINTERLACER_DEINIT,      0, NULL, NULL},
#endif
#endif
};
static DVP_U32 numRemoteKernels = dimof(remote_kernels);

static DVP_Function_t remote_functions[] = {
    {"DVP_SKEL_KernelGraphManagerInit", NULL, 0},
    {"DVP_SKEL_KernelGraphManagerDeinit", NULL, 0},
    {"DVP_SKEL_KernelGraphManager", NULL, 0},
};
static DVP_U32 numRemoteFunctions = dimof(remote_functions);

DVP_U32 DVP_KernelGraphManagerCallback(DVP_U32 numArgs, DVP_U32 *args)
{
    numArgs = numArgs; // warnings
    args = args; // warnings
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG": %u, %p\n", numArgs, args);
    return 0;
}

static DVP_Function_t local_functions[] = {
    {"DVP_KernelGraphManagerCallback", DVP_KernelGraphManagerCallback, 0},
};
static DVP_U32 numLocalFunctions = dimof(local_functions);

MODULE_EXPORT DVP_U32 DVP_GetSupportedKernels(DVP_CoreFunction_t **pFuncs)
{
    if (pFuncs != NULL)
        *pFuncs = remote_kernels;
    DVP_PRINT(DVP_ZONE_KGM, "Retreiving "KGM_TAG" Kernel Information!\n");
#ifdef DVP_USE_IPC
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_IPC enabled!\n");
#endif
#ifdef DVP_USE_VRUN
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_VRUN enabled!\n");
#endif
    return numRemoteKernels;
}

MODULE_EXPORT DVP_U32 DVP_GetSupportedRemoteCalls(DVP_Function_t **pRemote, DVP_U32 *pNum)
{
    *pRemote = remote_functions;
    *pNum = numRemoteFunctions;
    return 1;
}

MODULE_EXPORT DVP_U32 DVP_GetSupportedLocalCalls(DVP_Function_t **pLocal, DVP_U32 *pNum)
{
    *pLocal = local_functions;
    *pNum = numLocalFunctions;
    return 1;
}

MODULE_EXPORT DVP_Core_e DVP_GetSupportedRemoteCore()
{
    return DVP_CORE_SIMCOP;
}

MODULE_EXPORT DVP_U32 DVP_GetMaximumLoad(void)
{
#if defined(TARGET_DVP_OMAP4) && defined(BLAZE_TABLET)
    return 400; // 400 Mhz
#elif defined(TARGET_DVP_OMAP4)
    return 200; // 200 Mhz
#else
    return 0;
#endif
}

static DVP_RPC_t *rpc;
//static DVP_RPC_Interface_t coreInterface;
static DVP_RPC_Core_t *core;
static thread_t worker;
static queue_t *workqueue;
static queue_t *retqueue;
static semaphore_t coreLock;
static DVP_Perf_t perf;
static DVP_RPC_Translation_t translations;
static fourcc_t colorY800[] = {FOURCC_Y800};
static fourcc_t colorY16[] = {FOURCC_Y16};

static DVP_U32 DVP_KernelGraphManager_SIMCOP(DVP_KernelNode_t *pNodes, DVP_U32 startNode, DVP_U32 numNodes)
{
    DVP_U32 processed = 0;

    // block multiple entry
    semaphore_wait(&coreLock);

    if (pNodes)
    {
#if defined(DVP_USE_OMAPRPC)
        DVP_KernelNode_t *pTmp = pNodes; // temp var
#elif defined(DVP_USE_RCM)
        DVP_KernelNode_t *pTmp = &pNodes[startNode]; // this will hold the translated address
#endif

#ifdef DVP_USE_IPC
        DVP_U32 n = 0;
        int status;
        DVP_U32 size;

        DVP_PerformanceStart(&perf);

#if defined(DVP_USE_RCM)
        size = sizeof(DVP_KernelNode_t *);
#elif defined(DVP_USE_OMAPRPC)
        size = (startNode+numNodes)*sizeof(DVP_KernelNode_t);
#endif
        // @NOTE this implicitly sets the index variable to zero, which is correct in our situation.
        memset(&translations, 0, sizeof(translations));

        DVP_RPC_Parameter_t params[] = {
            {DVP_RPC_READ_ONLY, size, (DVP_PTR)&pTmp, (DVP_PTR)dvp_rpc_get_handle(rpc, pTmp)},
#if defined(DVP_USE_OMAPRPC)
            {DVP_RPC_READ_ONLY, sizeof(DVP_U32), (DVP_PTR)&startNode, NULL},
#endif
            {DVP_RPC_READ_ONLY, sizeof(DVP_U32), (DVP_PTR)&numNodes, NULL},
        };

        if (rpc == NULL || core == NULL)
        {
            DVP_PRINT(DVP_ZONE_ERROR, KGM_TAG": ERROR! Kernel Graph Manager for SIMCOP is uninitialized!\n");
            goto leave;
        }

        DVP_PRINT(DVP_ZONE_KGM, KGM_TAG": Executing %u nodes on subgraph %p! Base %p start %u\n", numNodes, pTmp, pNodes, startNode);

        // for each node, translate all the pointers
        for (n = startNode; n < (startNode + numNodes); n++)
        {
            DVP_PRINT(DVP_ZONE_KGM, KGM_TAG": Translating Node[%u] Kernel %u\n", n, pNodes[n].header.kernel);
            switch (pNodes[n].header.kernel)
            {
                case DVP_KN_NOOP:
#ifdef DVP_USE_DEI
                case DVP_KN_DEI_DEINTERLACER_INIT:
                case DVP_KN_DEI_DEINTERLACER_DEINIT:
#endif
                {
                    break;
                }
                case DVP_KN_COPY:
                {
                    DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIO->input);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIO->output);
                    break;
                }
#ifdef DVP_USE_VRUN
                case DVP_KN_DILATE_CROSS:
                case DVP_KN_DILATE_SQUARE:
                case DVP_KN_DILATE_MASK:
                case DVP_KN_ERODE_CROSS:
                case DVP_KN_ERODE_MASK:
                case DVP_KN_ERODE_SQUARE:
                case DVP_KN_VRUN_DILATE_CROSS:
                case DVP_KN_VRUN_DILATE_SQUARE:
                case DVP_KN_VRUN_DILATE_MASK:
                case DVP_KN_VRUN_ERODE_CROSS:
                case DVP_KN_VRUN_ERODE_MASK:
                case DVP_KN_VRUN_ERODE_SQUARE:
                {
                    DVP_Morphology_t *pMorph = dvp_knode_to(&pNodes[n], DVP_Morphology_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);

                    DVP_PrintImage(DVP_ZONE_KGM, &pMorph->input);
                    DVP_PrintImage(DVP_ZONE_KGM, &pMorph->output);

                    if (pNodes[n].header.kernel == DVP_KN_ERODE_MASK ||
                        pNodes[n].header.kernel == DVP_KN_DILATE_MASK ||
                        pNodes[n].header.kernel == DVP_KN_VRUN_ERODE_MASK ||
                        pNodes[n].header.kernel == DVP_KN_VRUN_DILATE_MASK)
                    {
                        dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->mask, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                        DVP_PrintImage(DVP_ZONE_KGM, &pMorph->mask);
                    }
                    break;
                }
                case DVP_KN_IIR_HORZ:
                case DVP_KN_IIR_VERT:
                case DVP_KN_VRUN_IIR_HORZ:
                case DVP_KN_VRUN_IIR_VERT:
                {
                    DVP_IIR_t *pIIR = dvp_knode_to(&pNodes[n], DVP_IIR_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIIR->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIIR->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIIR->scratch, DVP_FALSE, (DVP_PTR)pTmp, &translations);
#if !defined(__QNX__) /// @todo REMOVE: Temporary to avoid build error in QNX
                    if (pIIR->bounds[0].pData)
                    {
#if defined(DVP_USE_RCM)
                        DVP_COMPLAIN_IF_FALSE(dvp_rpc_flush(rpc, DVP_GetSupportedRemoteCore(), (DVP_PTR)pIIR->bounds[0].pData, pIIR->bounds[0].height*pIIR->bounds[0].width,DVP_MTYPE_MPUCACHED_VIRTUAL));
                        pIIR->bounds[0].pData = dvp_rpc_mem_xlate_fwrd(rpc, DVP_GetSupportedRemoteCore(), pIIR->bounds[0].pData, pIIR->bounds[0].height*pIIR->bounds[0].width, DVP_MTYPE_MPUCACHED_VIRTUAL);
#else
                        translations.translations[translations.numTranslations].offset = (DVP_U32)&pIIR->bounds[0].pData - (DVP_U32)pTmp;
                        translations.translations[translations.numTranslations].base = (DVP_U32)pIIR->bounds[0].pData;
                        translations.translations[translations.numTranslations].cacheops = DVP_TRUE;
                        translations.translations[translations.numTranslations].reserved = (DVP_PTR)dvp_rpc_get_handle(rpc, pIIR->bounds[0].pData);
                        translations.numTranslations++;
#endif
                    }
                    if (pIIR->bounds[1].pData)
                    {
#if defined(DVP_USE_RCM)
                        DVP_COMPLAIN_IF_FALSE(dvp_rpc_flush(rpc, DVP_GetSupportedRemoteCore(), (DVP_PTR)pIIR->bounds[1].pData, pIIR->bounds[1].height*pIIR->bounds[1].width,DVP_MTYPE_MPUCACHED_VIRTUAL));
                        pIIR->bounds[1].pData = dvp_rpc_mem_xlate_fwrd(rpc, DVP_GetSupportedRemoteCore(), pIIR->bounds[1].pData, pIIR->bounds[1].height*pIIR->bounds[1].width, DVP_MTYPE_MPUCACHED_VIRTUAL);
#else
                        translations.translations[translations.numTranslations].offset = (DVP_U32)&pIIR->bounds[1].pData - (DVP_U32)pTmp;
                        translations.translations[translations.numTranslations].base = (DVP_U32)pIIR->bounds[1].pData;
                        translations.translations[translations.numTranslations].cacheops = DVP_TRUE;
                        translations.translations[translations.numTranslations].reserved = (DVP_PTR)dvp_rpc_get_handle(rpc, pIIR->bounds[1].pData);
                        translations.numTranslations++;
#endif
                    }
#endif
                    DVP_PrintImage(DVP_ZONE_KGM, &pIIR->input);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIIR->output);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIIR->scratch);
                    break;
                }
                case DVP_KN_NONMAXSUPPRESS_3x3_S16:
                case DVP_KN_NONMAXSUPPRESS_5x5_S16:
                case DVP_KN_NONMAXSUPPRESS_7x7_S16:
                case DVP_KN_THR_GT2MAX_8:
                case DVP_KN_THR_GT2MAX_16:
                case DVP_KN_THR_GT2THR_8:
                case DVP_KN_THR_GT2THR_16:
                case DVP_KN_THR_LE2MIN_8:
                case DVP_KN_THR_LE2MIN_16:
                case DVP_KN_THR_LE2THR_8:
                case DVP_KN_THR_LE2THR_16:
                case DVP_KN_VRUN_NONMAXSUPPRESS_3x3_S16:
                case DVP_KN_VRUN_NONMAXSUPPRESS_5x5_S16:
                case DVP_KN_VRUN_NONMAXSUPPRESS_7x7_S16:
                case DVP_KN_VRUN_THR_GT2MAX_8:
                case DVP_KN_VRUN_THR_GT2MAX_16:
                case DVP_KN_VRUN_THR_GT2THR_8:
                case DVP_KN_VRUN_THR_GT2THR_16:
                case DVP_KN_VRUN_THR_LE2MIN_8:
                case DVP_KN_VRUN_THR_LE2MIN_16:
                case DVP_KN_VRUN_THR_LE2THR_8:
                case DVP_KN_VRUN_THR_LE2THR_16:
                {
                    DVP_Threshold_t *pThresh = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pThresh->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pThresh->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_CANNY_IMAGE_SMOOTHING:
                case DVP_KN_CONV_3x3:
                case DVP_KN_CONV_5x5:
                case DVP_KN_CONV_7x7:
                case DVP_KN_VRUN_CANNY_IMAGE_SMOOTHING:
                case DVP_KN_VRUN_CONV_3x3:
                case DVP_KN_VRUN_CONV_5x5:
                case DVP_KN_VRUN_CONV_7x7:
                case DVP_KN_VRUN_CONV_MxN:
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->mask, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VRUN_SAD_8x8:
                case DVP_KN_VRUN_SAD_16x16:
                case DVP_KN_VRUN_SAD_3x3:
                case DVP_KN_VRUN_SAD_5x5:
                case DVP_KN_VRUN_SAD_7x7:
                {
                    DVP_SAD_t *pImg = dvp_knode_to(&pNodes[n], DVP_SAD_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->refImg, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_CANNY_2D_GRADIENT:
                case DVP_KN_VRUN_CANNY_2D_GRADIENT:
                {
                    DVP_Canny2dGradient_t *pGrad = dvp_knode_to(&pNodes[n], DVP_Canny2dGradient_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->outGradX, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->outGradY, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->outMag, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_CANNY_NONMAX_SUPPRESSION:
                case DVP_KN_VRUN_CANNY_NONMAX_SUPPRESSION:
                {
                    DVP_CannyNonMaxSuppression_t *pCnonmax = dvp_knode_to(&pNodes[n], DVP_CannyNonMaxSuppression_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inMag, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inGradX, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inGradY, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_CANNY_HYST_THRESHHOLD:
                case DVP_KN_VRUN_CANNY_HYST_THRESHHOLD:
                {
                    DVP_CannyHystThresholding_t *pDth = dvp_knode_to(&pNodes[n], DVP_CannyHystThresholding_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inMag, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inEdgeMap, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_XYXY_TO_Y800: // single planes
                case DVP_KN_UYVY_TO_YUV444p: // multiplane output
                case DVP_KN_UYVY_TO_RGBp: // multiplane output
                case DVP_KN_UYVY_TO_YUV420p: // multiplane output
                case DVP_KN_INTEGRAL_IMAGE_8:
                case DVP_KN_NV12_TO_YUV444p:
                case DVP_KN_YUV444p_TO_RGBp:
                case DVP_KN_SOBEL_3x3_8s:
                case DVP_KN_SOBEL_3x3_8:
                case DVP_KN_SOBEL_3x3_16s:
                case DVP_KN_SOBEL_3x3_16:
                case DVP_KN_SOBEL_5x5_8s:
                case DVP_KN_SOBEL_5x5_8:
                case DVP_KN_SOBEL_5x5_16s:
                case DVP_KN_SOBEL_5x5_16:
                case DVP_KN_SOBEL_7x7_8s:
                case DVP_KN_SOBEL_7x7_8:
                case DVP_KN_SOBEL_7x7_16s:
                case DVP_KN_SOBEL_7x7_16:
                case DVP_KN_VRUN_XYXY_TO_Y800: // single planes
                case DVP_KN_VRUN_UYVY_TO_YUV444p: // multiplane output
                case DVP_KN_VRUN_UYVY_TO_RGBp: // multiplane output
                case DVP_KN_VRUN_UYVY_TO_YUV420p: // multiplane output
                case DVP_KN_VRUN_INTEGRAL_IMAGE_8:
                case DVP_KN_VRUN_NV12_TO_YUV444p:
                case DVP_KN_VRUN_YUV444p_TO_RGBp:
                case DVP_KN_VRUN_SOBEL_3x3_8s:
                case DVP_KN_VRUN_SOBEL_3x3_8:
                case DVP_KN_VRUN_SOBEL_3x3_16s:
                case DVP_KN_VRUN_SOBEL_3x3_16:
                case DVP_KN_VRUN_SOBEL_5x5_8s:
                case DVP_KN_VRUN_SOBEL_5x5_8:
                case DVP_KN_VRUN_SOBEL_5x5_16s:
                case DVP_KN_VRUN_SOBEL_5x5_16:
                case DVP_KN_VRUN_SOBEL_7x7_8s:
                case DVP_KN_VRUN_SOBEL_7x7_8:
                case DVP_KN_VRUN_SOBEL_7x7_16s:
                case DVP_KN_VRUN_SOBEL_7x7_16:
                case DVP_KN_LDC_AFFINE_TRANSFORM:
                {
                    DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIO->input);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIO->output);
                    break;
                }
                case DVP_KN_LDC_DISTORTION_CORRECTION:
                case DVP_KN_LDC_DISTORTION_AND_AFFINE:
                {
                    DVP_Ldc_t *pIO = dvp_knode_to(&pNodes[n], DVP_Ldc_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pIO->ldcLut, DVP_TRUE, (DVP_PTR)pTmp, &translations);

                    DVP_PrintImage(DVP_ZONE_KGM, &pIO->input);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIO->output);
                    break;
                }
                case DVP_KN_VRUN_HARRIS_CORNERS:
                {
                    DVP_HarrisCorners_t *pIO = dvp_knode_to(&pNodes[n], DVP_HarrisCorners_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->tempBuf1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->tempBuf2, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->tempBuf3, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->tempBuf4, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VRUN_HARRIS_SCORE_7x7:
                {

                    DVP_Harris_t *pIO = dvp_knode_to(&pNodes[n], DVP_Harris_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->harrisScore, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VRUN_BLOCK_MAXIMA:
                {
                    DVP_BlockMaxima_t *pIO  = dvp_knode_to(&pNodes[n], DVP_BlockMaxima_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->blockMaximaX, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->blockMaximaY, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->blockMaximaMAX, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VRUN_NMS_STEP1:
                {
                    DVP_NMSStep1_t *pIO  = dvp_knode_to(&pNodes[n], DVP_NMSStep1_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->nmsStep1X, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->nmsStep1Y, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->nmsStep1Dst, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VRUN_GAUSSIAN_3x3_PYRAMID_8:
                case DVP_KN_VRUN_GAUSSIAN_5x5_PYRAMID_8:
                case DVP_KN_VRUN_GAUSSIAN_7x7_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_H3x3_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_H5x5_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_H7x7_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_V3x3_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_V5x5_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_V7x7_PYRAMID_8:
                {
                    DVP_Transform_t *pGrad = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VRUN_IMAGE_PYRAMID_8:
                {
                    DVP_Pyramid_t *pPyramid = dvp_knode_to(&pNodes[n], DVP_Pyramid_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pPyramid->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pPyramid->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
#endif
                default:
                    return 0; // other kernels are not supported.
                    break;
            }
        }

#if defined(DVP_USE_RCM)
        // flush all the node data back to memory
        DVP_COMPLAIN_IF_FALSE(dvp_rpc_flush(rpc, DVP_GetSupportedRemoteCore(), (DVP_PTR)&pNodes[startNode], numNodes * sizeof(DVP_KernelNode_t), DVP_MTYPE_KERNELGRAPH));

        // now pTmp will be the translated value of the Node pointer on the remote core.
        pTmp = dvp_rpc_mem_xlate_fwrd(rpc, DVP_GetSupportedRemoteCore(), pNodes, numNodes*sizeof(DVP_KernelNode_t), DVP_MTYPE_KERNELGRAPH);

        // now increment to the node to be executed
        pTmp = &pTmp[startNode];
#endif
        // execute the remote functions
        RPC_COMPLAIN_IF_FAILED(status,dvp_rpc_remote_execute(rpc, core, DVP_KGM_REMOTE_EXEC, params, dimof(params), &translations));
        if (status >= 0)
            processed = status; // get the number of nodes from the "status".

#if defined(DVP_USE_RCM)
        // need to invalidate the kernel nodes, they may have been modified by the remote core (local data)
        DVP_COMPLAIN_IF_FALSE(dvp_rpc_invalidate(rpc, DVP_GetSupportedRemoteCore(), (DVP_PTR)&pNodes[startNode], numNodes * sizeof(DVP_KernelNode_t), DVP_MTYPE_KERNELGRAPH));
#endif

        // for each node, translate the node data back to local VM.
        for (n = startNode; n < (startNode + numNodes); n++)
        {
            switch (pNodes[n].header.kernel)
            {
                case DVP_KN_NOOP:
#ifdef DVP_USE_DEI
                case DVP_KN_DEI_DEINTERLACER_INIT:
                case DVP_KN_DEI_DEINTERLACER_DEINIT:
#endif
                {
                    break;
                }
                case DVP_KN_COPY:
                {
                    DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_TRUE);
                    break;
                }
#ifdef DVP_USE_VRUN
                case DVP_KN_DILATE_CROSS:
                case DVP_KN_DILATE_SQUARE:
                case DVP_KN_DILATE_MASK:
                case DVP_KN_ERODE_CROSS:
                case DVP_KN_ERODE_MASK:
                case DVP_KN_ERODE_SQUARE:
                case DVP_KN_VRUN_DILATE_CROSS:
                case DVP_KN_VRUN_DILATE_SQUARE:
                case DVP_KN_VRUN_DILATE_MASK:
                case DVP_KN_VRUN_ERODE_CROSS:
                case DVP_KN_VRUN_ERODE_MASK:
                case DVP_KN_VRUN_ERODE_SQUARE:
                {
                    DVP_Morphology_t *pMorph = dvp_knode_to(&pNodes[n], DVP_Morphology_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->output, DVP_TRUE);
                    if (pNodes[n].header.kernel == DVP_KN_ERODE_MASK ||
                        pNodes[n].header.kernel == DVP_KN_DILATE_MASK ||
                        pNodes[n].header.kernel == DVP_KN_VRUN_ERODE_MASK ||
                        pNodes[n].header.kernel == DVP_KN_VRUN_DILATE_MASK)
                    {
                        dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->mask, DVP_FALSE);
                    }
                    break;
                }
                case DVP_KN_IIR_HORZ:
                case DVP_KN_IIR_VERT:
                case DVP_KN_VRUN_IIR_HORZ:
                case DVP_KN_VRUN_IIR_VERT:
                {
                    DVP_IIR_t *pIIR = dvp_knode_to(&pNodes[n], DVP_IIR_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIIR->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIIR->output, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIIR->scratch, DVP_FALSE);
                    if (pIIR->bounds[0].pData)
                        pIIR->bounds[0].pData = dvp_rpc_mem_xlate_back(rpc, DVP_GetSupportedRemoteCore(), pIIR->bounds[0].pData, DVP_MTYPE_MPUCACHED_VIRTUAL);
                    if (pIIR->bounds[1].pData)
                        pIIR->bounds[1].pData = dvp_rpc_mem_xlate_back(rpc, DVP_GetSupportedRemoteCore(), pIIR->bounds[1].pData, DVP_MTYPE_MPUCACHED_VIRTUAL);
                    break;
                }
                case DVP_KN_NONMAXSUPPRESS_3x3_S16:
                case DVP_KN_NONMAXSUPPRESS_5x5_S16:
                case DVP_KN_NONMAXSUPPRESS_7x7_S16:
                case DVP_KN_THR_GT2MAX_8:
                case DVP_KN_THR_GT2MAX_16:
                case DVP_KN_THR_GT2THR_8:
                case DVP_KN_THR_GT2THR_16:
                case DVP_KN_THR_LE2MIN_8:
                case DVP_KN_THR_LE2MIN_16:
                case DVP_KN_THR_LE2THR_8:
                case DVP_KN_THR_LE2THR_16:
                case DVP_KN_VRUN_NONMAXSUPPRESS_3x3_S16:
                case DVP_KN_VRUN_NONMAXSUPPRESS_5x5_S16:
                case DVP_KN_VRUN_NONMAXSUPPRESS_7x7_S16:
                case DVP_KN_VRUN_THR_GT2MAX_8:
                case DVP_KN_VRUN_THR_GT2MAX_16:
                case DVP_KN_VRUN_THR_GT2THR_8:
                case DVP_KN_VRUN_THR_GT2THR_16:
                case DVP_KN_VRUN_THR_LE2MIN_8:
                case DVP_KN_VRUN_THR_LE2MIN_16:
                case DVP_KN_VRUN_THR_LE2THR_8:
                case DVP_KN_VRUN_THR_LE2THR_16:
                {
                    DVP_Threshold_t *pThresh = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pThresh->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pThresh->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_CANNY_IMAGE_SMOOTHING:
                case DVP_KN_CONV_3x3:
                case DVP_KN_CONV_5x5:
                case DVP_KN_CONV_7x7:
                case DVP_KN_VRUN_CANNY_IMAGE_SMOOTHING:
                case DVP_KN_VRUN_CONV_3x3:
                case DVP_KN_VRUN_CONV_5x5:
                case DVP_KN_VRUN_CONV_7x7:
                case DVP_KN_VRUN_CONV_MxN:
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->output, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->mask, DVP_FALSE);
                    break;
                }
                case DVP_KN_VRUN_SAD_8x8:
                case DVP_KN_VRUN_SAD_16x16:
                case DVP_KN_VRUN_SAD_3x3:
                case DVP_KN_VRUN_SAD_5x5:
                case DVP_KN_VRUN_SAD_7x7:
                {
                    DVP_SAD_t *pImg = dvp_knode_to(&pNodes[n], DVP_SAD_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->output, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->refImg, DVP_FALSE);
                    break;
                }
                case DVP_KN_CANNY_2D_GRADIENT:
                case DVP_KN_VRUN_CANNY_2D_GRADIENT:
                {
                    DVP_Canny2dGradient_t *pGrad = dvp_knode_to(&pNodes[n], DVP_Canny2dGradient_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->outGradX, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->outGradY, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->outMag, DVP_TRUE);
                    break;
                }
                case DVP_KN_CANNY_NONMAX_SUPPRESSION:
                case DVP_KN_VRUN_CANNY_NONMAX_SUPPRESSION:
                {
                    DVP_CannyNonMaxSuppression_t *pCnonmax = dvp_knode_to(&pNodes[n], DVP_CannyNonMaxSuppression_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inMag, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inGradX, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inGradY, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_CANNY_HYST_THRESHHOLD:
                case DVP_KN_VRUN_CANNY_HYST_THRESHHOLD:
                {
                    DVP_CannyHystThresholding_t *pDth = dvp_knode_to(&pNodes[n], DVP_CannyHystThresholding_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inMag, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inEdgeMap, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_XYXY_TO_Y800: // single planes
                case DVP_KN_UYVY_TO_YUV444p: // multiplane output
                case DVP_KN_UYVY_TO_RGBp: // multiplane output
                case DVP_KN_UYVY_TO_YUV420p:
                case DVP_KN_INTEGRAL_IMAGE_8:
                case DVP_KN_NV12_TO_YUV444p:
                case DVP_KN_YUV444p_TO_RGBp:
                case DVP_KN_SOBEL_3x3_8s:
                case DVP_KN_SOBEL_3x3_8:
                case DVP_KN_SOBEL_3x3_16s:
                case DVP_KN_SOBEL_3x3_16:
                case DVP_KN_SOBEL_5x5_8s:
                case DVP_KN_SOBEL_5x5_8:
                case DVP_KN_SOBEL_5x5_16s:
                case DVP_KN_SOBEL_5x5_16:
                case DVP_KN_SOBEL_7x7_8s:
                case DVP_KN_SOBEL_7x7_8:
                case DVP_KN_SOBEL_7x7_16s:
                case DVP_KN_SOBEL_7x7_16:
                case DVP_KN_VRUN_XYXY_TO_Y800: // single planes
                case DVP_KN_VRUN_UYVY_TO_YUV444p: // multiplane output
                case DVP_KN_VRUN_UYVY_TO_RGBp: // multiplane output
                case DVP_KN_VRUN_UYVY_TO_YUV420p:
                case DVP_KN_VRUN_INTEGRAL_IMAGE_8:
                case DVP_KN_VRUN_NV12_TO_YUV444p:
                case DVP_KN_VRUN_YUV444p_TO_RGBp:
                case DVP_KN_VRUN_SOBEL_3x3_8s:
                case DVP_KN_VRUN_SOBEL_3x3_8:
                case DVP_KN_VRUN_SOBEL_3x3_16s:
                case DVP_KN_VRUN_SOBEL_3x3_16:
                case DVP_KN_VRUN_SOBEL_5x5_8s:
                case DVP_KN_VRUN_SOBEL_5x5_8:
                case DVP_KN_VRUN_SOBEL_5x5_16s:
                case DVP_KN_VRUN_SOBEL_5x5_16:
                case DVP_KN_VRUN_SOBEL_7x7_8s:
                case DVP_KN_VRUN_SOBEL_7x7_8:
                case DVP_KN_VRUN_SOBEL_7x7_16s:
                case DVP_KN_VRUN_SOBEL_7x7_16:
                case DVP_KN_LDC_AFFINE_TRANSFORM:
                {
                    DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_LDC_DISTORTION_CORRECTION:
                case DVP_KN_LDC_DISTORTION_AND_AFFINE:
                {
                    DVP_Ldc_t *pIO = dvp_knode_to(&pNodes[n], DVP_Ldc_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pIO->ldcLut, DVP_FALSE);
                    break;
                }
                case DVP_KN_VRUN_HARRIS_CORNERS:
                {

                    DVP_HarrisCorners_t *pIO = dvp_knode_to(&pNodes[n], DVP_HarrisCorners_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->tempBuf1, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->tempBuf2, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->tempBuf3, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->tempBuf4, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_VRUN_HARRIS_SCORE_7x7:
                {
                    DVP_Harris_t *pIO = dvp_knode_to(&pNodes[n], DVP_Harris_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->harrisScore, DVP_TRUE);
                    break;
                }
                case DVP_KN_VRUN_BLOCK_MAXIMA:
                {
                    DVP_BlockMaxima_t *pIO  = dvp_knode_to(&pNodes[n], DVP_BlockMaxima_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->blockMaximaX, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->blockMaximaY, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->blockMaximaMAX, DVP_TRUE);
                    break;
                }
                case DVP_KN_VRUN_NMS_STEP1:
                {
                    DVP_NMSStep1_t *pIO  = dvp_knode_to(&pNodes[n], DVP_NMSStep1_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->nmsStep1X, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->nmsStep1Y, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->nmsStep1Dst, DVP_TRUE);
                    break;
                }
                case DVP_KN_VRUN_GAUSSIAN_3x3_PYRAMID_8:
                case DVP_KN_VRUN_GAUSSIAN_5x5_PYRAMID_8:
                case DVP_KN_VRUN_GAUSSIAN_7x7_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_H3x3_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_H5x5_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_H7x7_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_V3x3_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_V5x5_PYRAMID_8:
                case DVP_KN_VRUN_GRADIENT_V7x7_PYRAMID_8:
                {
                    DVP_Transform_t *pGrad = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_VRUN_IMAGE_PYRAMID_8:
                {
                    DVP_Pyramid_t *pPyramid = dvp_knode_to(&pNodes[n], DVP_Pyramid_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pPyramid->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pPyramid->output, DVP_TRUE);
                    break;
                }
#endif
                default:
                    goto leave;
            }
        }

        DVP_PerformanceStop(&perf);

#endif // DVP_USE_IPC

        DVP_PRINT(DVP_ZONE_KGM, KGM_TAG": Processed %u nodes!\n", processed);
    }
leave:
    semaphore_post(&coreLock); // allow other threads to enter
    return processed;
}

static thread_ret_t DVP_KernelGraphManagerThread_SIMCOP(void *arg __attribute__((unused)))
{
    DVP_KGM_Thread_t kgmt;

    thread_nextaffinity();
    while (queue_read(workqueue, true_e, &kgmt) == true_e)
    {
        DVP_KernelNode_t *pSubNodes = kgmt.pSubNodes;
        DVP_U32 startNode = kgmt.startNode;
        DVP_U32 numNodes = kgmt.numNodes;
        DVP_U32 processed = 0;

        processed = DVP_KernelGraphManager_SIMCOP(pSubNodes, startNode, numNodes);

        kgmt.numNodesExecuted = processed;
        queue_write(retqueue, true_e, &kgmt);
    }
    thread_exit(0);
}

MODULE_EXPORT void DVP_KernelGraphManagerRestart(void *arg __attribute__((unused)))
{
    int status = 0;
    DVP_U32 version = DVP_VERSION;
    DVP_U16 width = 320;
    DVP_U16 height = 240;
    DVP_RPC_Parameter_t params[] = {
        {DVP_RPC_READ_ONLY, sizeof(DVP_U16), &width, NULL},
        {DVP_RPC_READ_ONLY, sizeof(DVP_U16), &height, NULL},
        {DVP_RPC_READ_ONLY, sizeof(DVP_U32), &version, NULL},
    };
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" re-issuing init command to remote core.\n");
#ifdef DVP_USE_IPC
    RPC_COMPLAIN_IF_FAILED(status,dvp_rpc_remote_execute(rpc, core, DVP_KGM_REMOTE_INIT, params, dimof(params), NULL));
#endif
}

MODULE_EXPORT DVP_BOOL DVP_KernelGraphManagerDeinit(void)
{
    queue_pop(workqueue);
    queue_pop(retqueue);
    thread_join(worker);
    queue_destroy(workqueue);
    queue_destroy(retqueue);
    semaphore_delete(&coreLock);
    DVP_PerformancePrint(&perf, KGM_TAG);

    //For now the deinit is done automatically when the omaprpc driver
    //file descriptor is closed
#if defined(DVP_USE_IPC) && !defined(DVP_USE_OMAPRPC)
    int status = 0;
    DVP_PRINT(DVP_ZONE_KGM, "Destroying "KGM_TAG"\n");
    RPC_COMPLAIN_IF_FAILED(status,dvp_rpc_remote_execute(rpc, core, DVP_KGM_REMOTE_DEINIT, NULL, 0, NULL));
    core = NULL;
    rpc = NULL;
    if (status < 0)
        return DVP_FALSE;
    else
        return DVP_TRUE;
#else
    rpc = NULL;
    core = NULL;
    return DVP_TRUE;
#endif
}

MODULE_EXPORT DVP_U32 DVP_KernelGraphManagerInit(DVP_RPC_t *pRPC, DVP_RPC_Core_t *pCore)
{
    if (rpc == NULL && core == NULL && pRPC != NULL && pCore != NULL)
    {
        int status;
        //DVP_U32 version = DVP_VERSION;
        DVP_U32 width = 320;
        DVP_U32 height = 240;
        DVP_RPC_Parameter_t params[] = {
            {DVP_RPC_READ_ONLY, sizeof(DVP_U32), &width, NULL},
            {DVP_RPC_READ_ONLY, sizeof(DVP_U32), &height, NULL},
            //{DVP_RPC_READ_ONLY, sizeof(DVP_U32), &version, NULL}, // optional?
        };
        rpc = pRPC;
        core = pCore;
#ifdef DVP_USE_IPC
        RPC_COMPLAIN_IF_FAILED(status,dvp_rpc_remote_execute(rpc, core, DVP_KGM_REMOTE_INIT, params, dimof(params), NULL));
        if (status < 0)
            return DVP_FALSE;
        else
#endif
        {
            DVP_Perf_Clear(&perf);
            semaphore_create(&coreLock, 1, false_e);
            workqueue = queue_create(10, sizeof(DVP_KGM_Thread_t));
            retqueue  = queue_create(10, sizeof(DVP_KGM_Thread_t));
            worker = thread_create(DVP_KernelGraphManagerThread_SIMCOP, NULL);
            return DVP_TRUE;
        }
    }
    else
        return DVP_FALSE;
}

MODULE_EXPORT DVP_U32 DVP_KernelGraphManager(DVP_KernelNode_t *pSubNodes, DVP_U32 startNode, DVP_U32 numNodes, DVP_BOOL sync)
{
    DVP_PRINT(DVP_ZONE_KGM, "Entered "KGM_TAG" Kernel Manager! (%s)\n",(sync?"SYNC":"QUEUED"));
    if (sync == DVP_FALSE)
    {
        DVP_KGM_Thread_t kgmt = {pSubNodes, startNode, numNodes, 0};
        if (queue_write(workqueue, true_e, &kgmt) == true_e) // this is internally a copy
        {
            do {
                DVP_KGM_Thread_t kgmr;

                if (queue_read(retqueue, true_e, &kgmr) == true_e)
                {
                    if (kgmt.pSubNodes == kgmr.pSubNodes &&
                        kgmt.startNode == kgmr.startNode &&
                        kgmt.numNodes  == kgmr.numNodes) // this is ours, return.
                    {
                        DVP_PRINT(DVP_ZONE_KGM, "Work Thread returned %u nodes!\n", kgmr.numNodesExecuted);
                        return kgmr.numNodesExecuted;
                    }
                    else // it's not ours, put it back in the queue
                        queue_write(retqueue, true_e, &kgmr);
                }
                else
                    break;
            } while (1);
        }
        return 0;
    }
    else
    {
        return DVP_KernelGraphManager_SIMCOP(pSubNodes, startNode, numNodes);
    }
}

MODULE_EXPORT DVP_U32 DVP_KernelGraphManagerVerify(DVP_KernelNode_t *pNodes,
                                                   DVP_U32 startNode,
                                                   DVP_U32 numNodes)
{
    DVP_U32 n;
    DVP_U32 verified = 0;
    for (n = startNode; n < startNode + numNodes; n++)
    {
        // assume it will pass then set errors if detected.
        pNodes[n].header.error = DVP_SUCCESS;

        // check each supported kernel
        switch (pNodes[n].header.kernel)
        {
            case DVP_KN_NOOP:
#ifdef DVP_USE_DEI
            case DVP_KN_DEI_DEINTERLACER_INIT:
            case DVP_KN_DEI_DEINTERLACER_DEINIT:
#endif
            {
                break;
            }
            case DVP_KN_COPY:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 1, 1, 1, 1, &pIO->output.color, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 1, 1, 1, 1, &pIO->input.color, 1) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#ifdef DVP_USE_VRUN
            case DVP_KN_DILATE_CROSS:
            case DVP_KN_DILATE_SQUARE:
            case DVP_KN_DILATE_MASK:
            case DVP_KN_ERODE_CROSS:
            case DVP_KN_ERODE_MASK:
            case DVP_KN_ERODE_SQUARE:
            case DVP_KN_VRUN_DILATE_CROSS:
            case DVP_KN_VRUN_DILATE_SQUARE:
            case DVP_KN_VRUN_DILATE_MASK:
            case DVP_KN_VRUN_ERODE_CROSS:
            case DVP_KN_VRUN_ERODE_MASK:
            case DVP_KN_VRUN_ERODE_SQUARE:
            {
                DVP_Morphology_t *pMorph = dvp_knode_to(&pNodes[n], DVP_Morphology_t);
                if (DVP_Image_Validate(&pMorph->input, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pMorph->output, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    pMorph->input.width  > pMorph->output.width ||
                    pMorph->input.height > pMorph->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (pNodes[n].header.kernel == DVP_KN_ERODE_MASK ||
                    pNodes[n].header.kernel == DVP_KN_DILATE_MASK ||
                    pNodes[n].header.kernel == DVP_KN_VRUN_ERODE_MASK ||
                    pNodes[n].header.kernel == DVP_KN_VRUN_DILATE_MASK)
                {
                    if (DVP_Image_Validate(&pMorph->mask, 4, 1, 1, 1, colorY800, 1) == DVP_FALSE)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                }
                break;
            }
            case DVP_KN_IIR_HORZ:
            case DVP_KN_IIR_VERT:
            case DVP_KN_VRUN_IIR_HORZ:
            case DVP_KN_VRUN_IIR_VERT:
            {
                DVP_IIR_t *pIIR = dvp_knode_to(&pNodes[n], DVP_IIR_t);
                if (DVP_Image_Validate(&pIIR->input, 16, 16, 16, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIIR->output, 16, 16, 16, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIIR->scratch, 16, 16, 16, 2, colorY800, 1) == DVP_FALSE ||
                    pIIR->input.width  > pIIR->output.width ||
                    pIIR->input.height > pIIR->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_THR_GT2MAX_8:
            case DVP_KN_THR_GT2THR_8:
            case DVP_KN_THR_LE2MIN_8:
            case DVP_KN_THR_LE2THR_8:
            case DVP_KN_VRUN_THR_GT2MAX_8:
            case DVP_KN_VRUN_THR_GT2THR_8:
            case DVP_KN_VRUN_THR_LE2MIN_8:
            case DVP_KN_VRUN_THR_LE2THR_8:
            {
                DVP_Threshold_t *pThresh = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                if (DVP_Image_Validate(&pThresh->input, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pThresh->output, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    pThresh->input.width  > pThresh->output.width ||
                    pThresh->input.height > pThresh->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_THR_GT2MAX_16:
            case DVP_KN_THR_GT2THR_16:
            case DVP_KN_THR_LE2MIN_16:
            case DVP_KN_THR_LE2THR_16:
            case DVP_KN_VRUN_THR_GT2MAX_16:
            case DVP_KN_VRUN_THR_GT2THR_16:
            case DVP_KN_VRUN_THR_LE2MIN_16:
            case DVP_KN_VRUN_THR_LE2THR_16:
            {
                DVP_Threshold_t *pThresh = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                if (DVP_Image_Validate(&pThresh->input, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pThresh->output, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    pThresh->input.width  > pThresh->output.width ||
                    pThresh->input.height > pThresh->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_NONMAXSUPPRESS_3x3_S16:
            case DVP_KN_NONMAXSUPPRESS_5x5_S16:
            case DVP_KN_NONMAXSUPPRESS_7x7_S16:
            case DVP_KN_VRUN_NONMAXSUPPRESS_3x3_S16:
            case DVP_KN_VRUN_NONMAXSUPPRESS_5x5_S16:
            case DVP_KN_VRUN_NONMAXSUPPRESS_7x7_S16:
            {
                DVP_Threshold_t *pThresh = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                if (DVP_Image_Validate(&pThresh->input, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pThresh->output, 16, 16, 16, 2, colorY800, 1) == DVP_FALSE ||
                    pThresh->input.width  > pThresh->output.width ||
                    pThresh->input.height > pThresh->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                // Check for maximum widths in the case of SIMCOP
                if (pNodes[n].header.kernel == DVP_KN_NONMAXSUPPRESS_7x7_S16 ||
                    pNodes[n].header.kernel == DVP_KN_VRUN_NONMAXSUPPRESS_7x7_S16) {
                    if (pThresh->input.width > 768)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                } else {
                    if (pThresh->input.width > 992)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                }
                break;
            }
            case DVP_KN_CANNY_IMAGE_SMOOTHING:
            case DVP_KN_CONV_3x3:
            case DVP_KN_CONV_5x5:
            case DVP_KN_CONV_7x7:
            case DVP_KN_VRUN_CANNY_IMAGE_SMOOTHING:
            case DVP_KN_VRUN_CONV_3x3:
            case DVP_KN_VRUN_CONV_5x5:
            case DVP_KN_VRUN_CONV_7x7:
            case DVP_KN_VRUN_CONV_MxN:
            {
                DVP_ImageConvolution_t *pImg = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_t);
                if (DVP_Image_Validate(&pImg->input, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->mask, 4, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    pImg->input.width  > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VRUN_SAD_8x8:
            case DVP_KN_VRUN_SAD_16x16:
            case DVP_KN_VRUN_SAD_3x3:
            case DVP_KN_VRUN_SAD_5x5:
            case DVP_KN_VRUN_SAD_7x7:
            {
                DVP_SAD_t *pImg = dvp_knode_to(&pNodes[n], DVP_SAD_t);
                if (DVP_Image_Validate(&pImg->input, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->refImg, 16, 8, 8, 2, colorY800, 1) == DVP_FALSE ||
                    pImg->input.width  > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_CANNY_2D_GRADIENT:
            case DVP_KN_VRUN_CANNY_2D_GRADIENT:
            {
                DVP_Canny2dGradient_t *pGrad = dvp_knode_to(&pNodes[n], DVP_Canny2dGradient_t);
                if (DVP_Image_Validate(&pGrad->input, 16, 16, 16, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pGrad->outGradX, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pGrad->outGradY, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pGrad->outMag, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE ||
                    pGrad->input.width  > pGrad->outGradX.width ||
                    pGrad->input.height > pGrad->outGradX.height ||
                    pGrad->input.width  > pGrad->outGradY.width ||
                    pGrad->input.height > pGrad->outGradY.height ||
                    pGrad->input.width  > pGrad->outMag.width ||
                    pGrad->input.height > pGrad->outMag.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_CANNY_NONMAX_SUPPRESSION:
            case DVP_KN_VRUN_CANNY_NONMAX_SUPPRESSION:
            {
                DVP_CannyNonMaxSuppression_t *pCnonmax = dvp_knode_to(&pNodes[n], DVP_CannyNonMaxSuppression_t);
                if (DVP_Image_Validate(&pCnonmax->inMag, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCnonmax->inGradX, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCnonmax->inGradY, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCnonmax->output, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    pCnonmax->output.width  < pCnonmax->inMag.width ||
                    pCnonmax->output.height < pCnonmax->inMag.height ||
                    pCnonmax->output.width  < pCnonmax->inGradX.width ||
                    pCnonmax->output.height < pCnonmax->inGradX.height ||
                    pCnonmax->output.width  < pCnonmax->inGradY.width ||
                    pCnonmax->output.height < pCnonmax->inGradY.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_CANNY_HYST_THRESHHOLD:
            case DVP_KN_VRUN_CANNY_HYST_THRESHHOLD:
            {
                DVP_CannyHystThresholding_t *pDth = dvp_knode_to(&pNodes[n], DVP_CannyHystThresholding_t);
                if (DVP_Image_Validate(&pDth->inMag, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pDth->inEdgeMap, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pDth->output, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    pDth->output.width  < pDth->inMag.width ||
                    pDth->output.height < pDth->inMag.height ||
                    pDth->output.width  < pDth->inEdgeMap.width ||
                    pDth->output.height < pDth->inEdgeMap.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_SOBEL_3x3_8s:
            case DVP_KN_SOBEL_3x3_8:
            case DVP_KN_SOBEL_5x5_8s:
            case DVP_KN_SOBEL_5x5_8:
            case DVP_KN_SOBEL_7x7_8s:
            case DVP_KN_SOBEL_7x7_8:
            case DVP_KN_VRUN_SOBEL_3x3_8s:
            case DVP_KN_VRUN_SOBEL_3x3_8:
            case DVP_KN_VRUN_SOBEL_5x5_8s:
            case DVP_KN_VRUN_SOBEL_5x5_8:
            case DVP_KN_VRUN_SOBEL_7x7_8s:
            case DVP_KN_VRUN_SOBEL_7x7_8:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_SOBEL_3x3_16s:
            case DVP_KN_SOBEL_3x3_16:
            case DVP_KN_SOBEL_5x5_16s:
            case DVP_KN_SOBEL_5x5_16:
            case DVP_KN_SOBEL_7x7_16s:
            case DVP_KN_SOBEL_7x7_16:
            case DVP_KN_VRUN_SOBEL_3x3_16s:
            case DVP_KN_VRUN_SOBEL_3x3_16:
            case DVP_KN_VRUN_SOBEL_5x5_16s:
            case DVP_KN_VRUN_SOBEL_5x5_16:
            case DVP_KN_VRUN_SOBEL_7x7_16s:
            case DVP_KN_VRUN_SOBEL_7x7_16:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_XYXY_TO_Y800:
            case DVP_KN_VRUN_XYXY_TO_Y800:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t colorUYVY[] = {FOURCC_UYVY, FOURCC_VYUY};
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorUYVY, 2) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_INTEGRAL_IMAGE_8:
            case DVP_KN_VRUN_INTEGRAL_IMAGE_8:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t colorOut[] = {FOURCC_Y32};
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 32, 2, colorOut, 1) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (pIO->input.width > 2049 ||
                    pIO->input.height > 819)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_YUV444p:
            case DVP_KN_VRUN_UYVY_TO_YUV444p:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t colorIn[] = {FOURCC_UYVY};
                fourcc_t colorOut[] = {FOURCC_YV24, FOURCC_YU24};
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorIn, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 32, 2, colorOut, 2) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_RGBp:
            case DVP_KN_VRUN_UYVY_TO_RGBp:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t colorIn[] = {FOURCC_UYVY};
                fourcc_t colorOut[] = {FOURCC_RGBP};
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorIn, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 32, 2, colorOut, 1) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_YUV420p:
            case DVP_KN_VRUN_UYVY_TO_YUV420p:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t colorIn[] = {FOURCC_UYVY};
                fourcc_t colorOut[] = {FOURCC_IYUV, FOURCC_YV12};
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorIn, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 32, 2, colorOut, 2) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_NV12_TO_YUV444p:
            case DVP_KN_VRUN_NV12_TO_YUV444p:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t colorIn[] = {FOURCC_NV12};
                fourcc_t colorOut[] = {FOURCC_YV24, FOURCC_YU24};
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorIn, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 32, 2, colorOut, 2) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width*2 ||
                    pIO->input.height > pIO->output.height*2)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YUV444p_TO_RGBp:
            case DVP_KN_VRUN_YUV444p_TO_RGBp:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t colorIn[] = {FOURCC_YU24, FOURCC_YV24};
                fourcc_t colorOut[] = {FOURCC_RGBP};
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorIn, 2) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 32, 2, colorOut, 1) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_LDC_AFFINE_TRANSFORM:
            case DVP_KN_LDC_DISTORTION_CORRECTION:
            case DVP_KN_LDC_DISTORTION_AND_AFFINE:
            {
                DVP_Ldc_t *pIO = dvp_knode_to(&pNodes[n], DVP_Ldc_t);
                fourcc_t colorLDC[] = {FOURCC_UYVY, FOURCC_NV12};
                if (DVP_Image_Validate(&pIO->input, 16, 16, 16, 2, colorLDC, 2) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 16, 2, colorLDC, 2) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (pNodes[n].header.kernel != DVP_KN_LDC_AFFINE_TRANSFORM &&
                    DVP_Buffer_Validate(&pIO->ldcLut) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                 break;
            }
            case DVP_KN_VRUN_HARRIS_CORNERS:
            {
                DVP_HarrisCorners_t *pIO = dvp_knode_to(&pNodes[n], DVP_HarrisCorners_t);
                if (DVP_Image_Validate(&pIO->input, 16, 16, 16, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->tempBuf1, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->tempBuf2, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->tempBuf3, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->tempBuf4, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 16, 2, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width  > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VRUN_HARRIS_SCORE_7x7:
            {
                DVP_Harris_t *pIO = dvp_knode_to(&pNodes[n], DVP_Harris_t);
                if (DVP_Image_Validate(&pIO->input, 16, 16, 16, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->harrisScore, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VRUN_BLOCK_MAXIMA:
            {
                DVP_BlockMaxima_t *pIO  = dvp_knode_to(&pNodes[n], DVP_BlockMaxima_t);
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->blockMaximaX, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->blockMaximaY, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->blockMaximaMAX, 16, 16, 32, 2, colorY16, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VRUN_NMS_STEP1:
            {
                DVP_NMSStep1_t *pIO  = dvp_knode_to(&pNodes[n], DVP_NMSStep1_t);
                if (DVP_Image_Validate(&pIO->nmsStep1X, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->nmsStep1Y, 16, 16, 16, 2, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->nmsStep1Dst, 16, 16, 16, 2, colorY800, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VRUN_GAUSSIAN_3x3_PYRAMID_8:
            case DVP_KN_VRUN_GAUSSIAN_5x5_PYRAMID_8:
            case DVP_KN_VRUN_GAUSSIAN_7x7_PYRAMID_8:
            case DVP_KN_VRUN_GRADIENT_H3x3_PYRAMID_8:
            case DVP_KN_VRUN_GRADIENT_H5x5_PYRAMID_8:
            case DVP_KN_VRUN_GRADIENT_H7x7_PYRAMID_8:
            case DVP_KN_VRUN_GRADIENT_V3x3_PYRAMID_8:
            case DVP_KN_VRUN_GRADIENT_V5x5_PYRAMID_8:
            case DVP_KN_VRUN_GRADIENT_V7x7_PYRAMID_8:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 16, 16, 32, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 16, 16, 16, 2, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width  > (pIO->output.width*2) ||
                    pIO->input.height > (pIO->output.height*2))
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VRUN_IMAGE_PYRAMID_8:
            {
                DVP_Pyramid_t *pPyramid = dvp_knode_to(&pNodes[n], DVP_Pyramid_t);
                if (DVP_Image_Validate(&pPyramid->input, 16, 16, 128, 2, colorY800, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pPyramid->output) == DVP_FALSE ||
                    pPyramid->output.numBytes < pPyramid->input.width*pPyramid->input.height*21/64)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
            default:
                pNodes[n].header.error = DVP_ERROR_NOT_IMPLEMENTED;
                break;
        }
        if (pNodes[n].header.error == DVP_SUCCESS)
            verified++;
    }
    return verified;
}

#if defined(SYSBIOS_SL)
static static_function_t dvp_kgm_functions[] = {
    {"DVP_KernelGraphManagerInit",   (function_f)DVP_KernelGraphManagerInit},
    {"DVP_KernelGraphManager",       (function_f)DVP_KernelGraphManager},
    {"DVP_GetSupportedKernels",      (function_f)DVP_GetSupportedKernels},
    {"DVP_GetSupportedLocalCalls",   (function_f)DVP_GetSupportedLocalCalls},
    {"DVP_GetSupportedRemoteCalls",  (function_f)DVP_GetSupportedRemoteCalls},
    {"DVP_GetSupportedRemoteCore",   (function_f)DVP_GetSupportedRemoteCore},
    {"DVP_GetMaximumLoad",           (function_f)DVP_GetMaximumLoad},
    {"DVP_KernelGraphManagerDeinit", (function_f)DVP_KernelGraphManagerDeinit},
    {"DVP_KernelGraphManagerVerify", (function_f)DVP_KernelGraphManagerVerify},
};

static_module_table_t dvp_kgm_simcop_table = {
    "dvp_kgm_simcop",
    dimof(dvp_kgm_functions),
    dvp_kgm_functions
};
#endif
