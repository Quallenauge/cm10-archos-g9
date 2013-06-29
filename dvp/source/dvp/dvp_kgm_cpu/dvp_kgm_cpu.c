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

#include <sosal/sosal.h>

#include <dvp/dvp.h>
#include <dvp_kgm.h>
#include <dvp/dvp_debug.h>

#include <sosal/pyuv.h>

#if defined(DVP_USE_VLIB)
#include <vlib/vlib.h>
#include <vlib/dvp_kl_vlib.h>
#include <vlib/dvp_kgm_vlib.h>
#endif

#if defined(DVP_USE_TILER)
#undef PAGE_SIZE // this is to remove a warning
#include <tilermem_utils.h>
#endif

// ARMv7 Optimized Routines
#define DVP_OPTIMIZED_KERNELS
#include <dvp_ll.h>

#if defined(DVP_USE_YUV)
#include <yuv/yuv_armv7.h>
#include <yuv/dvp_kl_yuv.h>
#endif

#if defined(DVP_USE_IMGFILTER)
#include <imgfilter/imgFilter_armv7.h>
#include <imgfilter/dvp_kl_imgfilter.h>
#endif

#if defined(DVP_USE_RVM)
#include <rvm/RVM_wrapper.h>
#include <rvm/dvp_kl_rvm.h>
#endif

#if defined(DVP_USE_IMGLIB)
#include <imglib/imglib64plus.h>
#include <imglib/dvp_kl_imglib.h>
#endif

#if defined(DVP_USE_ORB)
#include <orb/cORB.h>
#include <orb/dvp_kl_orb.h>
static DVP_U08 *orbMem;
static DVP_U08 orbInit;
static CORBInput * orbInput;
#endif

#define KGM_TAG "DVP KGM CPU"

#if defined(DVP_USE_VLIB) || defined(DVP_USE_IMGLIB)
static DVP_S16 yuv2rgbMatrix[5] = {8192, 11229, -2757, -5720, 14192};
#endif

#if defined(DVP_USE_VLIB)
static S16 saturate_s16(S32 v)
{
    if (v > MAX_S16)
        v = MAX_S16;
    if (v < -MAX_S16)
        v = -MAX_S16;
    return v;
}
#endif

#if defined(DVP_OPTIMIZED_KERNELS) && defined(DVP_USE_YUV)
typedef enum _dvp_optimized_kernels_e {
    DVP_KN_OPT_UYVY_TO_RGBp_LUMA_YUV444p = DVP_KN_LOCAL_OPTIMIZED_BASE + 1,
    DVP_KN_OPT_UYVY_TO_RGBp_YUV444p_LUMA,
    DVP_KN_OPT_UYVY_TO_YUV444p_LUMA_RGBp,
    DVP_KN_OPT_UYVY_TO_YUV444p_RGBp_LUMA,
    DVP_KN_OPT_UYVY_TO_LUMA_YUV444p_RGBp,
    DVP_KN_OPT_UYVY_TO_LUMA_RGBp_YUV444p,
} DVP_OptimizedKernels_e;

typedef struct _dvp_optimized_kernels_t {
    DVP_OptimizedKernels_e  optkernel;
    DVP_ENUM                *kernels;
    DVP_U32                 numKernels;
} DVP_OptimizedKernels_t;

DVP_ENUM opt1[] = {DVP_KN_UYVY_TO_RGBp, DVP_KN_XYXY_TO_Y800, DVP_KN_UYVY_TO_YUV444p};
DVP_ENUM opt2[] = {DVP_KN_UYVY_TO_RGBp, DVP_KN_UYVY_TO_YUV444p, DVP_KN_XYXY_TO_Y800};
DVP_ENUM opt3[] = {DVP_KN_UYVY_TO_YUV444p, DVP_KN_XYXY_TO_Y800, DVP_KN_UYVY_TO_RGBp};
DVP_ENUM opt4[] = {DVP_KN_UYVY_TO_YUV444p, DVP_KN_UYVY_TO_RGBp, DVP_KN_XYXY_TO_Y800};
DVP_ENUM opt5[] = {DVP_KN_XYXY_TO_Y800, DVP_KN_UYVY_TO_YUV444p, DVP_KN_UYVY_TO_RGBp};
DVP_ENUM opt6[] = {DVP_KN_XYXY_TO_Y800, DVP_KN_UYVY_TO_RGBp, DVP_KN_UYVY_TO_YUV444p};

DVP_OptimizedKernels_t optkerns[] = {
    {DVP_KN_OPT_UYVY_TO_RGBp_LUMA_YUV444p, opt1, dimof(opt1)},
    {DVP_KN_OPT_UYVY_TO_RGBp_YUV444p_LUMA, opt2, dimof(opt2)},
    {DVP_KN_OPT_UYVY_TO_YUV444p_LUMA_RGBp, opt3, dimof(opt3)},
    {DVP_KN_OPT_UYVY_TO_YUV444p_RGBp_LUMA, opt4, dimof(opt4)},
    {DVP_KN_OPT_UYVY_TO_LUMA_YUV444p_RGBp, opt5, dimof(opt5)},
    {DVP_KN_OPT_UYVY_TO_LUMA_RGBp_YUV444p, opt6, dimof(opt6)},
};
DVP_U32 numOptKerns = dimof(optkerns);
#endif

#if (defined(DVP_USE_IMGLIB) || defined(DVP_USE_VLIB))
static dvp_image_shift_t cpu_shift3 = {
    -1, -1, 0, 2, 2, 0,
};
#endif

#if (defined(DVP_USE_IMGLIB) || defined(DVP_USE_VLIB))
static dvp_image_shift_t cpu_shift5 = {
    -2, -2, 0, 4, 4, 0,
};
#endif

#if (defined(DVP_USE_IMGLIB) /*|| defined(DVP_USE_VLIB)*/)
static dvp_image_shift_t cpu_shift7 = {
    -3, -3, 0, 6, 6, 0,
};
#endif

#if (defined(DVP_USE_IMGLIB) /*|| defined(DVP_USE_VLIB)*/)
static dvp_image_shift_t cpu_shift8 = {
    -4, -4, 0, 7, 7, 0,
};
#endif

#if (defined(DVP_USE_IMGLIB) /*|| defined(DVP_USE_VLIB)*/)
static dvp_image_shift_t cpu_shift11 = {
    -5, -5, 0, 10, 10, 0,
};
#endif

#if (defined(DVP_USE_IMGLIB) /*|| defined(DVP_USE_VLIB)*/)
static dvp_image_shift_t cpu_shift16 = {
    -8, -8, 0, 15, 15, 0,
};
#endif

#if (defined(DVP_USE_VLIB) /*|| defined(DVP_USE_IMGLIB) */)
static dvp_image_shift_t cpu_nonmax_shift3 = {
    -1, 1, 2, 2, 0, 0,
};
#endif

#if (defined(DVP_USE_VLIB) /*|| defined(DVP_USE_IMGLIB) */)
static dvp_image_shift_t cpu_nonmax_shift5 = {
    -2, 2, 4, 4, 0, 0,
};
#endif

#if (defined(DVP_USE_VLIB) /*|| defined(DVP_USE_IMGLIB) */)
static dvp_image_shift_t cpu_nonmax_shift7 = {
    -3, 3, 6, 6, 0, 0,
};
#endif

#if defined(DVP_USE_VLIB)
static void dvp_vlib_nms_mxn_shift(DVP_KernelNode_t *node, dvp_image_shift_t *shift)
{
    if (node && shift)
    {
        DVP_Nonmax_NxN_t *pNMS = dvp_knode_to(node, DVP_Nonmax_NxN_t);
        shift->centerShiftHorz = -(DVP_S32)pNMS->p/2;
        shift->centerShiftVert = -(DVP_S32)pNMS->p/2;
        shift->rightBorder = pNMS->p - 1;
        shift->bottomBorder = pNMS->p - 1;
    }
}
#endif

/*! \brief The list of locally supported kernels.
 * \note Please list features first, then algo library specific versions.
 * \note Also, please list any algo specific versions with the algo library
 * name in the description.
 */
static DVP_CoreFunction_t local_kernels[] = {
    // name, kernel, priority, load

    //***************************************
    // FEATURE LIST
    //***************************************

    {"No operation",   DVP_KN_NOOP, 0, NULL, NULL},
    {"Threshold",      DVP_KN_THRESHOLD, 0, NULL, NULL},
    {"XStrideConvert", DVP_KN_XSTRIDE_CONVERT, 0, NULL, NULL},
    {"XStrideShift",   DVP_KN_XSTRIDE_SHIFT, 0, NULL, NULL},
    {"Image Copy",     DVP_KN_COPY, 0, NULL, NULL},
    {"Image Debug",    DVP_KN_IMAGE_DEBUG, 0, NULL, NULL},
    {"Buffer Debug",   DVP_KN_BUFFER_DEBUG, 0, NULL, NULL},
    {"Gamma",          DVP_KN_GAMMA, 0, NULL, NULL},

#if defined(DVP_USE_YUV)
    {"NEON YXYX to LUMA", DVP_KN_YUV_YXYX_TO_Y800, 0, NULL, NULL},
#endif

#if defined(DVP_USE_YUV)
    {"NEON YXYX to LUMA", DVP_KN_YXYX_TO_Y800, 0, NULL, NULL},
#elif defined(DVP_USE_VLIB)
    {"\"C\" YXYX to LUMA", DVP_KN_YXYX_TO_Y800, 0, NULL, NULL},
#endif

#if defined(DVP_USE_VLIB)
    {"\"C\" DilateCross", DVP_KN_DILATE_CROSS, 0, &cpu_shift3, NULL},
    {"\"C\" DilateMask", DVP_KN_DILATE_MASK, 0, &cpu_shift3, NULL},
    {"\"C\" DilateSquare", DVP_KN_DILATE_SQUARE, 0, &cpu_shift3, NULL},
    {"\"C\" ErodeCross", DVP_KN_ERODE_CROSS, 0, &cpu_shift3, NULL},
    {"\"C\" ErodeMask", DVP_KN_ERODE_MASK, 0, &cpu_shift3, NULL},
    {"\"C\" ErodeSquare", DVP_KN_ERODE_SQUARE, 0, &cpu_shift3, NULL},

    {"\"C\" Canny2DGradient", DVP_KN_CANNY_2D_GRADIENT, 0, &cpu_shift3, NULL},
    {"\"C\" CannyNonmaxSupress", DVP_KN_CANNY_NONMAX_SUPPRESSION, 0, &cpu_shift3, NULL},
    {"\"C\" CannyHyst.Thresh", DVP_KN_CANNY_HYST_THRESHHOLD, 0, NULL, NULL},

    {"\"C\" IIRHorz", DVP_KN_IIR_HORZ, 0, NULL, NULL},
    {"\"C\" IIRVert", DVP_KN_IIR_VERT, 0, NULL, NULL},

    {"\"C\" IntegralImg8", DVP_KN_INTEGRAL_IMAGE_8, 0, NULL, NULL},

    {"\"C\" Nonmaxsupress3x316", DVP_KN_NONMAXSUPPRESS_3x3_S16, 0, &cpu_nonmax_shift3, NULL},
    {"\"C\" Nonmaxsupress5x516", DVP_KN_NONMAXSUPPRESS_5x5_S16, 0, &cpu_nonmax_shift5, NULL},
    {"\"C\" Nonmaxsupress7x716", DVP_KN_NONMAXSUPPRESS_7x7_S16, 0, &cpu_nonmax_shift7, NULL},

    {"\"C\" YUV422p to UYVY", DVP_KN_YUV422p_TO_UYVY, 0, NULL, NULL},
    {"\"C\" UYVY to YUV422p", DVP_KN_UYVY_TO_YUV422p, 0, NULL, NULL},
#endif

#if defined(DVP_USE_IMGLIB)
    {"\"C\" YUV420p to RGB565", DVP_KN_YUV422p_TO_RGB565, 0, NULL, NULL},
    {"\"C\" Sobel 3x3",    DVP_KN_SOBEL_3x3_8, 0, &cpu_shift3, NULL},
//    {"\"C\" Sobel 3x3_16s", DVP_KN_SOBEL_3x3_16s, 0, NULL, NULL}, //Removing due to mismatch with SIMCOP
//    {"\"C\" Sobel 5x5_16s", DVP_KN_SOBEL_5x5_16s, 0, NULL, NULL},
//    {"\"C\" Sobel 7x7_16s", DVP_KN_SOBEL_7x7_16s, 0, NULL, NULL},
    {"\"C\" Conv 3x3",     DVP_KN_CONV_3x3, 0, &cpu_shift3, NULL},
    {"\"C\" Conv 5x5",     DVP_KN_CONV_5x5, 0, &cpu_shift5, NULL},
    {"\"C\" Conv 7x7",     DVP_KN_CONV_7x7, 0, &cpu_shift7, NULL},

    {"\"C\" CannyImgSmooth",   DVP_KN_CANNY_IMAGE_SMOOTHING, 0, &cpu_shift7, NULL},

    {"\"C\" Thr gt2max8",  DVP_KN_THR_GT2MAX_8, 0, NULL, NULL},
    {"\"C\" Thr gt2max16", DVP_KN_THR_GT2MAX_16, 0, NULL, NULL},
    {"\"C\" Thr gt2thr8",  DVP_KN_THR_GT2THR_8, 0, NULL, NULL},
    {"\"C\" Thr gt2thr16", DVP_KN_THR_GT2THR_16, 0, NULL, NULL},
    {"\"C\" Thr le2min8",  DVP_KN_THR_LE2MIN_8, 0, NULL, NULL},
    {"\"C\" Thr le2min16", DVP_KN_THR_LE2MIN_16, 0, NULL, NULL},
    {"\"C\" Thr le2thr8",  DVP_KN_THR_LE2THR_8, 0, NULL, NULL},
    {"\"C\" Thr le2thr16", DVP_KN_THR_LE2THR_16, 0, NULL, NULL},
#endif

#if defined(DVP_USE_YUV)
    {"NEON xYxY to LUMA",    DVP_KN_XYXY_TO_Y800, 0, NULL, NULL},
    {"NEON UYVY to YUV420p", DVP_KN_UYVY_TO_YUV420p, 0, NULL, NULL},
    {"NEON UYVY to YUV444p", DVP_KN_UYVY_TO_YUV444p, 0, NULL, NULL},
    {"NEON UYVY to RGBp",    DVP_KN_UYVY_TO_RGBp, 0, NULL, NULL},
#elif defined(DVP_USE_VLIB)
    {"\"C\" xYxY to LUMA",    DVP_KN_XYXY_TO_Y800, 0, NULL, NULL},
    {"\"C\" UYVY to YUV420p", DVP_KN_UYVY_TO_YUV420p, 0, NULL, NULL},
    {"\"C\" UYVY to YUV444p", DVP_KN_UYVY_TO_YUV444p, 0, NULL, NULL},
    {"\"C\" UYVY to RGBp",    DVP_KN_UYVY_TO_RGBp, 0, NULL, NULL},
#endif

#if defined(DVP_USE_YUV)
    {"NEON NV12 to YUV444p Half Scale", DVP_KN_NV12_TO_YUV444p, 0, NULL, NULL},
    {"NEON BGR3 to UYVY",    DVP_KN_BGR3_TO_UYVY, 0, NULL, NULL},
    {"NEON BGR3 to IYUV",    DVP_KN_BGR3_TO_IYUV, 0, NULL, NULL},
#elif defined(DVP_USE_IMAGE)
    {"\"C\" NV12 to YUV444p", DVP_KN_NV12_TO_YUV444p, 0, NULL, NULL},
    {"\"C\" RGB3 to UYVY",    DVP_KN_BGR3_TO_UYVY, 0, NULL, NULL},
    {"\"C\" BGR3 to IYUV",    DVP_KN_BGR3_TO_IYUV, 0, NULL, NULL},
#endif

#if defined(DVP_USE_YUV)
    {"NEON YUV444 to RGBP",  DVP_KN_YUV444p_TO_RGBp, 0, NULL, NULL},
    {"NEON LUMA to xYxY",    DVP_KN_Y800_TO_XYXY, 0, NULL, NULL},
    {"NEON YUV420 to RGBp",  DVP_KN_YUV420p_TO_RGBp, 0, NULL, NULL},
    {"NEON UYVY to BGR",     DVP_KN_UYVY_TO_BGR, 0, NULL, NULL},
#endif

#if defined(DVP_USE_IMAGE)
    {"\"C\" YUV444p to UYVY ", DVP_KN_YUV444p_TO_UYVY, 0, NULL, NULL},
    {"\"C\" NV12 to UYVY",     DVP_KN_NV12_TO_UYVY, 0, NULL, NULL},
    {"\"C\" RGB3 to NV12",    DVP_KN_BGR3_TO_NV12, 0, NULL, NULL},
#endif


#if defined(DVP_USE_IMGFILTER)
    {"NEON Sobel3x3",   DVP_KN_SOBEL_8, 0, NULL, NULL},
    {"NEON Scharr3x3",  DVP_KN_SCHARR_8, 0, NULL, NULL},
    {"NEON Kroon3x3",   DVP_KN_KROON_8, 0, NULL, NULL},
    {"NEON Prewitt3x3", DVP_KN_PREWITT_8, 0, NULL, NULL},
#elif defined(DVP_USE_VLIB)
    {"\"C\" Sobel3x3",   DVP_KN_SOBEL_8, 0, NULL, NULL},
    {"\"C\" Scharr3x3",  DVP_KN_SCHARR_8, 0, NULL, NULL},
    {"\"C\" Kroon3x3",   DVP_KN_KROON_8, 0, NULL, NULL},
    {"\"C\" Prewitt3x3", DVP_KN_PREWITT_8, 0, NULL, NULL},
#endif

    //***************************************
    // ALGO LIBRARY SPECIFIC LIST
    //***************************************

#if defined(DVP_USE_IMGLIB)
    {"\"C\" IMGLIB YUV420p to RGB565", DVP_KN_IMG_YUV422p_TO_RGB565, 0, NULL, NULL},
    {"\"C\" IMGLIB Sobel 3x3",    DVP_KN_IMG_SOBEL_3x3_8, 0, &cpu_shift3, NULL},
    {"\"C\" IMGLIB sobel 3x3_16s", DVP_KN_IMG_SOBEL_3x3_16s, 0, &cpu_shift3, NULL},
    {"\"C\" IMGLIB sobel 5x5_16s", DVP_KN_IMG_SOBEL_5x5_16s, 0, &cpu_shift5, NULL},
    {"\"C\" IMGLIB sobel 7x7_16s", DVP_KN_IMG_SOBEL_7x7_16s, 0, &cpu_shift7, NULL},
    {"\"C\" IMGLIB Clipping 16",   DVP_KN_IMG_CLIPPING_16, 0, NULL, NULL},
    {"\"C\" IMGLIB Boundary8",     DVP_KN_IMG_BOUNDARY_8, 0, NULL, NULL},
    {"\"C\" IMGLIB Boundary16s",   DVP_KN_IMG_BOUNDARY_16s, 0, NULL, NULL},
    {"\"C\" IMGLIB Correlation 3x3", DVP_KN_IMG_CORR_3x3, 0, &cpu_shift3, NULL},
    {"\"C\" IMGLIB Correlation 3x3 I8 C16s", DVP_KN_IMG_CORR_3x3_I8_C16s, 0, &cpu_shift3, NULL},
    {"\"C\" IMGLIB Correlation 3x3 I16s C16s", DVP_KN_IMG_CORR_3x3_I16s_C16s, 0, &cpu_shift3, NULL},
    {"\"C\" IMGLIB Correlation 5x5 I16s C16s", DVP_KN_IMG_CORR_5x5_I16s_C16s, 0, &cpu_shift5, NULL},
    {"\"C\" IMGLIB Correlation 11x11 I8 C16s", DVP_KN_IMG_CORR_11x11_I8_C16s, 0, &cpu_shift11, NULL},
    {"\"C\" IMGLIB Correlation 11x11 I16s C16s", DVP_KN_IMG_CORR_11x11_I16s_C16s, 0, &cpu_shift11, NULL},
    {"\"C\" IMGLIB Correlation Gen I16 C16s", DVP_KN_IMG_CORR_GEN_I16s_C16s, 0, &cpu_shift11, NULL},
    {"\"C\" IMGLIB Correlation Gen Iq", DVP_KN_IMG_CORR_GEN_IQ, 0, NULL, NULL},
    {"\"C\" IMGLIB Histogram8",   DVP_KN_IMG_HISTOGRAM_8, 0, NULL, NULL},
    {"\"C\" IMGLIB Histogram16",  DVP_KN_IMG_HISTOGRAM_16, 0, NULL, NULL},
    {"\"C\" IMGLIB Median 3x3 8", DVP_KN_IMG_MEDIAN_3x3_8, 0, &cpu_shift3, NULL},
    {"\"C\" IMGLIB Median 3x3 16s", DVP_KN_IMG_MEDIAN_3x3_16s, 0, &cpu_shift3, NULL},
    {"\"C\" IMGLIB Demux LE 8",   DVP_KN_IMG_YC_DEMUX_LE16_8, 0, NULL, NULL},
    {"\"C\" IMGLIB Demux BE 8",   DVP_KN_IMG_YC_DEMUX_BE16_8, 0, NULL, NULL},
    {"\"C\" IMGLIB Pix Sat ",     DVP_KN_IMG_PIX_SAT, 0, NULL, NULL},
    {"\"C\" IMGLIB Pix Expand ",  DVP_KN_IMG_PIX_EXPAND, 0, NULL, NULL},
    {"\"C\" IMGLIB SAD 3x3",      DVP_KN_IMG_SAD_3x3, 0, &cpu_shift3, NULL},
    {"\"C\" IMGLIB SAD 5x5",      DVP_KN_IMG_SAD_5x5, 0, &cpu_shift5, NULL},
    {"\"C\" IMGLIB SAD 7x7",      DVP_KN_IMG_SAD_7x7, 0, &cpu_shift7, NULL},
    {"\"C\" IMGLIB SAD 8x8",      DVP_KN_IMG_SAD_8x8, 0, &cpu_shift8, NULL},
    {"\"C\" IMGLIB SAD 16x16",    DVP_KN_IMG_SAD_16x16, 0, &cpu_shift16, NULL},
    {"\"C\" IMGLIB Conv 3x3",     DVP_KN_IMG_CONV_3x3, 0, &cpu_shift3, NULL},
    {"\"C\" IMGLIB Conv 5x5",     DVP_KN_IMG_CONV_5x5, 0, &cpu_shift5, NULL},
    {"\"C\" IMGLIB Conv 7x7",     DVP_KN_IMG_CONV_7x7, 0, &cpu_shift7, NULL},
    {"\"C\" IMGLIB Conv 11x11",   DVP_KN_IMG_CONV_11x11, 0, &cpu_shift11, NULL},
    {"\"C\" IMGLIB Conv 5x5 i8 c16s", DVP_KN_IMG_CONV_5x5_I8_C16, 0, &cpu_shift5, NULL},
    {"\"C\" IMGLIB Conv 7x7 i8 c16s", DVP_KN_IMG_CONV_7x7_I8_C16, 0, &cpu_shift7, NULL},
    {"\"C\" IMGLIB Conv 3x3 i16s c16s", DVP_KN_IMG_CONV_3x3_I16s_C16, 0, &cpu_shift3, NULL},
    {"\"C\" IMGLIB Conv 5x5 i16 c16s", DVP_KN_IMG_CONV_5x5_I16s_C16, 0, &cpu_shift5, NULL},
    {"\"C\" IMGLIB Conv 7x7 i16 c16s", DVP_KN_IMG_CONV_7x7_I16s_C16, 0, &cpu_shift7, NULL},
    {"\"C\" IMGLIB Conv 11x11 i16s c16s", DVP_KN_IMG_CONV_11x11_I16s_C16, 0, &cpu_shift11, NULL},

    {"\"C\" IMGLIB Thr gt2max8",  DVP_KN_IMG_THR_GT2MAX_8, 0, NULL, NULL},
    {"\"C\" IMGLIB Thr gt2max16", DVP_KN_IMG_THR_GT2MAX_16, 0, NULL, NULL},
    {"\"C\" IMGLIB Thr gt2thr8",  DVP_KN_IMG_THR_GT2THR_8, 0, NULL, NULL},
    {"\"C\" IMGLIB Thr gt2thr16", DVP_KN_IMG_THR_GT2THR_16, 0, NULL, NULL},
    {"\"C\" IMGLIB Thr le2min8",  DVP_KN_IMG_THR_LE2MIN_8, 0, NULL, NULL},
    {"\"C\" IMGLIB Thr le2min16", DVP_KN_IMG_THR_LE2MIN_16, 0, NULL, NULL},
    {"\"C\" IMGLIB Thr le2thr8",  DVP_KN_IMG_THR_LE2THR_8, 0, NULL, NULL},
    {"\"C\" IMGLIB Thr le2thr16", DVP_KN_IMG_THR_LE2THR_16, 0, NULL, NULL},

    {"\"C\" IMGLIB Perimeter8",   DVP_KN_IMG_PERIMETER_8, 0, NULL, NULL},
    {"\"C\" IMGLIB Perimeter16",  DVP_KN_IMG_PERIMETER_16, 0, NULL, NULL},
    {"\"C\" IMGLIB ErrDiff 8",    DVP_KN_IMG_ERRDIFF_BIN_8, 0, NULL, NULL},
    {"\"C\" IMGLIB ErrDiff 16",   DVP_KN_IMG_ERRDIFF_BIN_16, 0, NULL, NULL},
    {"\"C\" IMGLIB FDCT 8x8",     DVP_KN_IMG_FDCT_8x8, 0, NULL, NULL},
    {"\"C\" IMGLIB IFDCT 8x8",    DVP_KN_IMG_IDCT_8x8_12Q4, 0, NULL, NULL},
    {"\"C\" IMGLIB MAD 8x8",      DVP_KN_IMG_MAD_8x8, 0, NULL, NULL},
    {"\"C\" IMGLIB MAD 16x16",    DVP_KN_IMG_MAD_16x16, 0, NULL, NULL},
    {"\"C\" IMGLIB Quantization 16", DVP_KN_IMG_QUANTIZE_16, 0, NULL, NULL},
    /*! \todo Support Wavelet Quant in CPU. */
    //{"\"C\" IMGLIB Wavelet Horz", DVP_KN_IMG_WAVE_HORZ, 0, NULL, NULL},
#endif

#if defined(DVP_USE_VLIB)
    {"\"C\" VLIB UYVY to HSLp",    DVP_KN_VLIB_UYVY_TO_HSLp, 0, NULL, NULL},
    {"\"C\" VLIB UYVY to LABp",    DVP_KN_VLIB_UYVY_TO_LABp, 0, NULL, NULL},
    {"\"C\" VLIB YUV422p to UYVY", DVP_KN_VLIB_YUV422p_TO_UYVY, 0, NULL, NULL},
    {"\"C\" VLIB EWRMeanS16", DVP_KN_VLIB_EWR_MEAN_S16, 0, NULL, NULL},
    {"\"C\" VLIB EWRVarS16", DVP_KN_VLIB_EWR_VAR_S16, 0, NULL, NULL},
    {"\"C\" VLIB EWRMeanS32", DVP_KN_VLIB_EWR_MEAN_S32, 0, NULL, NULL},
    {"\"C\" VLIB EWRVarS32", DVP_KN_VLIB_EWR_VAR_S32, 0, NULL, NULL},
    {"\"C\" VLIB UWRMeanS16", DVP_KN_VLIB_UWR_MEAN_S16, 0, NULL, NULL},
    {"\"C\" VLIB UWRVarS16", DVP_KN_VLIB_UWR_VAR_S16, 0, NULL, NULL},

    {"\"C\" VLIB BackSubS16", DVP_KN_VLIB_SUB_BACK_S16, 0, NULL, NULL},
    {"\"C\" VLIB BackSubS32", DVP_KN_VLIB_SUB_BACK_S32, 0, NULL, NULL},
    {"\"C\" VLIB MixofGaussS16", DVP_KN_VLIB_MOG_S16, 0, NULL, NULL},
    {"\"C\" VLIB MixofGaussS32", DVP_KN_VLIB_MOG_S32, 0, NULL, NULL},
    {"\"C\" VLIB ExtractBack16", DVP_KN_VLIB_EXTRACT_BACK_8_16, 0, NULL, NULL},

    {"\"C\" VLIB PackMask32", DVP_KN_VLIB_PACK_MASK_32, 0, NULL, NULL},
    {"\"C\" VLIB UnPackMask32", DVP_KN_VLIB_UNPACK_MASK_32, 0, NULL, NULL},

    {"\"C\" VLIB DilateCross", DVP_KN_VLIB_DILATE_CROSS, 0, &cpu_shift3, NULL},
    {"\"C\" VLIB DilateMask", DVP_KN_VLIB_DILATE_MASK, 0, &cpu_shift3, NULL},
    {"\"C\" VLIB DilateSquare", DVP_KN_VLIB_DILATE_SQUARE, 0, &cpu_shift3, NULL},
    {"\"C\" VLIB ErodeCross", DVP_KN_VLIB_ERODE_CROSS, 0, &cpu_shift3, NULL},
    {"\"C\" VLIB ErodeMask", DVP_KN_VLIB_ERODE_MASK, 0, &cpu_shift3, NULL},
    {"\"C\" VLIB ErodeSquare", DVP_KN_VLIB_ERODE_SQUARE, 0, &cpu_shift3, NULL},

    {"\"C\" VLIB Erode1Pixel", DVP_KN_VLIB_ERODE_SINGLEPIXEL, 0, NULL, NULL},

    {"\"C\" VLIB ConnectedComponent", DVP_KN_VLIB_CCL, 0, NULL, NULL},

    {"\"C\" VLIB Canny2DGradient", DVP_KN_VLIB_CANNY_2D_GRADIENT, 0, &cpu_shift3, NULL},
    {"\"C\" VLIB CannyNonmaxSupress", DVP_KN_VLIB_CANNY_NONMAX_SUPPRESSION, 0, &cpu_shift3, NULL},
    {"\"C\" VLIB CannyHyst.Thresh", DVP_KN_VLIB_CANNY_HYST_THRESHHOLD, 0, NULL, NULL},

    {"\"C\" VLIB ImgPyramid8", DVP_KN_VLIB_IMAGE_PYRAMID_8, 0, NULL, NULL},
    {"\"C\" VLIB ImgPyramid16", DVP_KN_VLIB_IMAGE_PYRAMID_16, 0, NULL, NULL},

    {"\"C\" VLIB Gauss5x5Pyramid8", DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_8, 0, &cpu_shift5, NULL},
    {"\"C\" VLIB Gauss5x5Pyramid16", DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_16, 0, &cpu_shift5, NULL},

    {"\"C\" VLIB GradientH5x5Pyramid8", DVP_KN_VLIB_GRADIENT_H5x5_PYRAMID_8, 0, &cpu_shift5, NULL},
    {"\"C\" VLIB GradientV5x5Pyramid8", DVP_KN_VLIB_GRADIENT_V5x5_PYRAMID_8, 0, &cpu_shift5, NULL},

    {"\"C\" VLIB HarrisScore", DVP_KN_VLIB_HARRIS_SCORE_7x7, 0, NULL, NULL},
    {"\"C\" VLIB HarrisScore7x7_U32", DVP_KN_VLIB_HARRIS_SCORE_7x7_U32, 0, NULL, NULL},

    {"\"C\" VLIB IIRHorz", DVP_KN_VLIB_IIR_HORZ, 0, NULL, NULL},
    {"\"C\" VLIB IIRHorz16", DVP_KN_VLIB_IIR_HORZ_16, 0, NULL, NULL},
    {"\"C\" VLIB IIRVert", DVP_KN_VLIB_IIR_VERT, 0, NULL, NULL},
    {"\"C\" VLIB IIRVert16", DVP_KN_VLIB_IIR_VERT_16, 0, NULL, NULL},

    {"\"C\" VLIB Integral Image 8", DVP_KN_VLIB_INTEGRAL_IMAGE_8, 0, NULL, NULL},
    {"\"C\" VLIB Integral Image 16", DVP_KN_VLIB_INTEGRAL_IMAGE_16, 0, NULL, NULL},

    {"\"C\" VLIB HoughLine", DVP_KN_VLIB_HOUGH_LINE_FROM_LIST, 0, NULL, NULL},
    {"\"C\" VLIB Hysterisis Threshold", DVP_KN_VLIB_HYST_THRESHOLD, 0, NULL, NULL},

    {"\"C\" VLIB Nonmaxsuppress 3x3 S16", DVP_KN_VLIB_NONMAXSUPPRESS_3x3_S16, 0, &cpu_nonmax_shift3, NULL},
    {"\"C\" VLIB Nonmaxsuppress 5x5 S16", DVP_KN_VLIB_NONMAXSUPPRESS_5x5_S16, 0, &cpu_nonmax_shift5, NULL},
    {"\"C\" VLIB Nonmaxsuppress 7x7 S16", DVP_KN_VLIB_NONMAXSUPPRESS_7x7_S16, 0, &cpu_nonmax_shift7, NULL},
    {"\"C\" VLIB Nonmaxsuppress NxN U32", DVP_KN_VLIB_NONMAXSUPPRESS_U32, 0, NULL, dvp_vlib_nms_mxn_shift},

    {"\"C\" VLIB NormalFlow 16", DVP_KN_VLIB_NORMALFLOW_16, 0, NULL, NULL},
    //{"\"C\" VLIB Kalman2x4", DVP_KN_VLIB_KALMAN_2x4, 0, NULL, NULL},
    //{"\"C\" VLIB Kalman4x6", DVP_KN_VLIB_KALMAN_4x6, 0, NULL, NULL},
    {"\"C\" VLIB Nelder-MeadSimplex16", DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_16, 0, NULL, NULL},
    {"\"C\" VLIB Nelder-MeanSimplex3D", DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_3D, 0, NULL, NULL},
    //{"\"C\" VLIB LegendreMoments", DVP_KN_VLIB_LEGENDRE_MOMENTS, 0, NULL, NULL},

    {"\"C\" VLIB InitHistgram8", DVP_KN_VLIB_INIT_HISTOGRAM_8, 0, NULL, NULL},
    {"\"C\" VLIB Histogram8", DVP_KN_VLIB_HISTOGRAM_8, 0, NULL, NULL},
    {"\"C\" VLIB InitHistogram16", DVP_KN_VLIB_INIT_HISTOGRAM_16, 0, NULL, NULL},
    {"\"C\" VLIB Histogram16", DVP_KN_VLIB_HISTOGRAM_16, 0, NULL, NULL},
    {"\"C\" VLIB WeightedHistogram8", DVP_KN_VLIB_WEIGHTED_HISTOGRAM_8, 0, NULL, NULL},
    {"\"C\" VLIB WeightedHistogram16", DVP_KN_VLIB_WEIGHTED_HISTOGRAM_16, 0, NULL, NULL},

    {"\"C\" VLIB HistogramnD16", DVP_KN_VLIB_HISTOGRAM_ND_16, 0, NULL, NULL},
    {"\"C\" VLIB BhattacharyyaDistance", DVP_KN_VLIB_BHATTACHAYA_DISTANCE, 0, NULL, NULL},

    {"\"C\" VLIB L1Distance", DVP_KN_VLIB_L1DISTANCE, 0, NULL, NULL},

    {"\"C\" VLIB Lucas Kanade Tracking", DVP_KN_VLIB_TRACK_FEATURES_LUCAS_7x7, 0, NULL, NULL},
#if defined(VLIB_DISPARITY_FIXED)
    {"\"C\" VLIB Disparity8", DVP_KN_VLIB_DISPARITY_SAD8, 0, NULL, NULL},
    {"\"C\" VLIB Disparity16", DVP_KN_VLIB_DISPARITY_SAD16, 0, NULL, NULL},
#endif
    {"\"C\" VLIB MeanLuma16", DVP_KN_VLIB_INIT_MEAN_LUMA_S16, 0, NULL, NULL},
    {"\"C\" VLIB MeanLuma32", DVP_KN_VLIB_INIT_MEAN_LUMA_S32, 0, NULL, NULL},
    {"\"C\" VLIB VarConst16", DVP_KN_VLIB_INIT_VAR_CONST_S16, 0, NULL, NULL},
    {"\"C\" VLIB VarConst32", DVP_KN_VLIB_INIT_VAR_CONST_S32, 0, NULL, NULL},

    {"\"C\" VLIB YxYx to LUMA",  DVP_KN_VLIB_YXYX_TO_Y800, 0, NULL, NULL},
    {"\"C\" VLIB YUV422p to UYVY", DVP_KN_VLIB_YUV422p_TO_UYVY, 0, NULL, NULL},
    {"\"C\" VLIB UYVY to YUV422p", DVP_KN_VLIB_UYVY_TO_YUV422p, 0, NULL, NULL},

#endif //DVP_USE_VLIB

#if defined(DVP_USE_YUV)
    {"NEON \"YUV\" xYxY to LUMA",    DVP_KN_YUV_XYXY_TO_Y800, 0, NULL, NULL},
    {"NEON \"YUV\" UYVY to YUV420p", DVP_KN_YUV_UYVY_TO_IYUV, 0, NULL, NULL},
    {"NEON \"YUV\" UYVY to YUV444p", DVP_KN_YUV_UYVY_TO_YUV444p, 0, NULL, NULL},
    {"NEON \"YUV\" UYVY to RGBp",    DVP_KN_YUV_UYVY_TO_RGBp, 0, NULL, NULL},
#elif defined(DVP_USE_VLIB)
    {"\"C\" VLIB xYxY to LUMA",  DVP_KN_VLIB_XYXY_TO_Y800, 0, NULL, NULL},
    {"\"C\" VLIB UYVY to YUV420p", DVP_KN_VLIB_UYVY_TO_YUV420p, 0, NULL, NULL},
    {"\"C\" VLIB UYVY to YUV444p", DVP_KN_VLIB_UYVY_TO_YUV444p, 0, NULL, NULL},
    {"\"C\" VLIB UYVY to RGBp", DVP_KN_VLIB_UYVY_TO_RGBp, 0, NULL, NULL},
#endif

#if defined(DVP_USE_YUV)
//    {"NEON \"YUV\" YUV444 to RGBp",  DVP_KN_YUV_YUV444_TO_RGBp, 0, NULL, NULL},
    {"NEON \"YUV\" LUMA to xYxY",    DVP_KN_YUV_Y800_TO_XYXY, 0, NULL, NULL},
    {"NEON \"YUV\" NV12 to YUV444p", DVP_KN_YUV_NV12_TO_YU24_HALF_SCALE, 0, NULL, NULL},
    {"NEON \"YUV\" UYVY HALF SCALE", DVP_KN_YUV_UYVY_HALF_SCALE, 0, NULL, NULL},
    {"NEON \"YUV\" UYVY QTR SCALE",  DVP_KN_YUV_UYVY_QTR_SCALE, 0, NULL, NULL},
    {"NEON \"YUV\" PLANAR ROTATE CW90", DVP_KN_YUV_Y800_ROTATE_CW_90, 0, NULL, NULL},
    {"NEON \"YUV\" PLANAR ROTATE CCW90", DVP_KN_YUV_Y800_ROTATE_CCW_90, 0, NULL, NULL},
    {"NEON \"YUV\" UYVY ROTATE CW90", DVP_KN_YUV_UYVY_ROTATE_CW_90, 0, NULL, NULL},
    {"NEON \"YUV\" UYVY ROTATE CCW90", DVP_KN_YUV_UYVY_ROTATE_CCW_90, 0, NULL, NULL},
    {"NEON \"YUV\" UYVY to BGR",     DVP_KN_YUV_UYVY_TO_BGR, 0, NULL, NULL},
    {"NEON \"YUV\" ARGB to UYVY",    DVP_KN_YUV_ARGB_TO_UYVY, 0, NULL, NULL},

    {"NEON \"YUV\" BGR3 to UYVY", DVP_KN_YUV_BGR_TO_UYVY, 0, NULL, NULL},
    {"NEON \"YUV\" BGR3 to IYUV", DVP_KN_YUV_BGR_TO_IYUV, 0, NULL, NULL},
#endif

#if defined(DVP_USE_IMGFILTER)
    {"NEON IMGFILTER Sobel3x3",   DVP_KN_IMGFILTER_SOBEL, 0, NULL, NULL},
    {"NEON IMGFILTER Scharr3x3",  DVP_KN_IMGFILTER_SCHARR, 0, NULL, NULL},
    {"NEON IMGFILTER Kroon3x3",   DVP_KN_IMGFILTER_KROON, 0, NULL, NULL},
    {"NEON IMGFILTER Prewitt3x3", DVP_KN_IMGFILTER_PREWITT, 0, NULL, NULL},
#elif defined(DVP_USE_VLIB)
    {"\"C\" VLIB Sobel3x3",   DVP_KN_VLIB_SOBEL_8, 0, NULL, NULL},
    {"\"C\" VLIB Scharr3x3",  DVP_KN_VLIB_SCHARR_8, 0, NULL, NULL},
    {"\"C\" VLIB Kroon3x3",   DVP_KN_VLIB_KROON_8, 0, NULL, NULL},
    {"\"C\" VLIB Prewitt3x3", DVP_KN_VLIB_PREWITT_8, 0, NULL, NULL},
#endif

#if defined(DVP_USE_RVM)
    {"\"C\" RVM", DVP_KN_RVM, 0, NULL, NULL},
#endif
#if defined(DVP_USE_ORB)
    {"\"C\" ORB", DVP_KN_ORB, 0, NULL, NULL},
#endif
};
static DVP_U32 numLocalKernels = dimof(local_kernels);

void gamma_apply(DVP_Image_t *input, DVP_Image_t *output, DVP_U08* gammaLut);

static DVP_BOOL DVP_Transform_Check(DVP_KernelNode_t *pNode, fourcc_t *from, DVP_U32 fromLen, fourcc_t *to, DVP_U32 toLen)
{
    DVP_Transform_t *pT = dvp_knode_to(pNode, DVP_Transform_t);
    if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, from, fromLen) == DVP_FALSE ||
        DVP_Image_Validate(&pT->output, 1, 1, 1, 1, to, toLen) == DVP_FALSE ||
        pT->input.width > pT->output.width ||
        pT->input.height > pT->output.height)
        return DVP_FALSE;
    return DVP_TRUE;
}

//******************************************************************************
// MODULE EXPORTS
//******************************************************************************

MODULE_EXPORT DVP_U32 DVP_GetSupportedKernels(DVP_CoreFunction_t **pFuncs)
{
    if (pFuncs != NULL)
        *pFuncs = local_kernels;
    DVP_PRINT(DVP_ZONE_KGM, "Retreiving "KGM_TAG" Kernel Information!\n");
#if defined(DVP_USE_IMGLIB)
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_IMGLIB enabled!\n");
#endif
#if defined(DVP_USE_VLIB)
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_VLIB enabled!\n");
#endif
#if defined(DVP_USE_YUV)
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_YUV enabled!\n");
#endif
#if defined(DVP_USE_IMAGE)
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_IMAGE enabled!\n");
#endif
#if defined(DVP_USE_RVM)
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_RVM enabled!\n");
#endif
    return numLocalKernels;
}

MODULE_EXPORT DVP_U32 DVP_GetSupportedRemoteCalls(DVP_Function_t **pRemote, DVP_U32 *pNum)
{
    *pRemote = 0;
    *pNum = 0;
    return 0;
}

MODULE_EXPORT DVP_U32 DVP_GetSupportedLocalCalls(DVP_Function_t **pLocal, DVP_U32 *pNum)
{
    *pLocal = 0;
    *pNum = 0;
    return 0;
}

MODULE_EXPORT DVP_Core_e DVP_GetSupportedRemoteCore()
{
    return DVP_CORE_CPU;
}

MODULE_EXPORT DVP_U32 DVP_GetMaximumLoad(void)
{
    return TARGET_NUM_CORES * 1000;
}

static thread_t workers[TARGET_NUM_CORES];
static queue_t *workqueue;
static queue_t *retqueue;

static void DVP_Image_to_image_t(image_t *img, DVP_Image_t *pImage)
{
    uint32_t p;
    memset(img, 0, sizeof(image_t));
    img->color = pImage->color;
    img->numPlanes = pImage->planes;
    for (p = 0; p < pImage->planes; p++)
    {
        img->plane[p].ptr = pImage->pData[p];
        img->plane[p].xdim = pImage->width;
        img->plane[p].ydim = pImage->height;
        img->plane[p].xstep = 1;
        img->plane[p].xscale = 1;
        img->plane[p].yscale = 1;
        img->plane[p].xstride = pImage->x_stride;
        img->plane[p].ystride = pImage->y_stride;
    }
    switch (img->color)
    {
        case FOURCC_BIN1:
            img->plane[0].xscale = 8;
            break;
        case FOURCC_UYVY:
        case FOURCC_YUY2:
        case FOURCC_VYUY:
        case FOURCC_YVYU:
            img->plane[0].xstep = 2;
            break;
        case FOURCC_NV12:
            img->plane[1].xstep = 2;
            img->plane[1].xscale = 2;
            img->plane[1].yscale = 2;
            break;
        case FOURCC_IYUV:
        case FOURCC_YV12:
            img->plane[1].xscale = 2;
            img->plane[2].xscale = 2;
            img->plane[1].yscale = 2;
            img->plane[2].yscale = 2;
            break;
        case FOURCC_YU16:
        case FOURCC_YV16:
            img->plane[1].xscale = 2;
            img->plane[2].xscale = 2;
            break;
        default:
            break;
    }
    image_print(img);
}

static DVP_U32 DVP_KernelGraphManager_CPU(DVP_KernelNode_t *pSubNodes, DVP_U32 startNode, DVP_U32 numNodes)
{
    DVP_U32 n,i = 0;
    DVP_S32 processed = 0;
    DVP_Perf_t *pPerf = NULL;
    DVP_ENUM kernel = 0;

    if (pSubNodes)
    {
        for (n = startNode; n < (startNode + numNodes); n++)
        {
            // assume it's not optimized
            kernel = pSubNodes[n].header.kernel;

#if defined(DVP_OPTIMIZED_KERNELS) && defined(DVP_USE_YUV)
            // check for optimized kernels
            for (i = 0; i < numOptKerns; i++)
            {
                // if the first kernel in the optimized chain matches
                if (optkerns[i].kernels[0] == kernel)
                {
                    DVP_U32 j, k;
                    DVP_PRINT(DVP_ZONE_KGM, "Kernel %u is possibly optimized by OptKernel %u\n", kernel, optkerns[i].optkernel);
                    // continue to check
                    for (j = 1, k = n+1; j < optkerns[i].numKernels && k < numNodes; j++,k++)
                    {
                        if (pSubNodes[k].header.kernel != optkerns[i].kernels[j])
                            break;
                    }
                    // do all kernels match?
                    if (j == optkerns[i].numKernels)
                    {
                        // match! reassign to optimized kernel
                        kernel = optkerns[i].optkernel;
                        // pre-increment index over to last relevant node
                        n += optkerns[i].numKernels - 1;
                        DVP_PRINT(DVP_ZONE_KGM, "Optimized Kernel %u will be used! It will save %u nodes! Moved to node %u\n", kernel, optkerns[i].numKernels,n);
                        break;
                    }
                }
            }
#endif

#ifdef DVP_DEBUG
            for (i = 0; i < dimof(local_kernels); i++)
            {
                if (local_kernels[i].kernel == (DVP_KernelNode_e)kernel) {
                    DVP_PRINT(DVP_ZONE_KGM, "Executing Kernel %s\n",local_kernels[i].name);
                }
            }
#endif
            // initialize the perf pointer and clock rate
            pPerf = &pSubNodes[n].header.perf;
            pPerf->rate = rtimer_freq(); // fill in the clock rate used to capture data.
            DVP_PerformanceStart(pPerf);

            // initialize the error status. Change to an error if one
            // occurs.
            pSubNodes[n].header.error = DVP_SUCCESS;

            switch (kernel)
            {
                case DVP_KN_NOOP:
                {
                    break;
                }
                case DVP_KN_COPY:
                {
                    DVP_Transform_t *pIO = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (DVP_Image_Copy(&pIO->output, &pIO->input) == DVP_FALSE)
                    {
                        processed--;
                        pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                    }
                    break;
                }
                case DVP_KN_THRESHOLD: // Assumes 8bits for now
                {
                    DVP_U32 j, p, y = 0;
                    DVP_U08 *tmpIn, *tmpOut;
                    DVP_Transform_t *pIO = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pIO->input.planes == pIO->output.planes &&
                        pIO->input.height == pIO->output.height &&
                        pIO->input.width == pIO->output.width &&
                        pIO->input.x_stride == pIO->output.x_stride)
                    {
                        for (p = 0; p < pIO->input.planes; p++)
                        {
                            for (y = 0; y < pIO->input.height; y++)
                            {
                                tmpIn  = DVP_Image_Addressing(&pIO->input, 0, y, p);
                                tmpOut = DVP_Image_Addressing(&pIO->output, 0, y, p);
                                for(j = 0; j < pIO->input.width; j++)
                                {
                                    tmpOut[j] = (tmpIn[j] >> 7);    // Make binary image.
                                }
                            }
                        }
                    }
                    else
                    {
                        processed--;
                        pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                    }
                    break;
                }
                case DVP_KN_XSTRIDE_CONVERT:
                case DVP_KN_XSTRIDE_SHIFT:
                {
                    DVP_U32 j, p, y = 0;
                    DVP_Transform_t *pIO = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pIO->input.planes == pIO->output.planes &&
                        pIO->input.height == pIO->output.height &&
                        pIO->input.width == pIO->output.width)
                    {
                        for (p = 0; p < pIO->input.planes; p++)
                        {
                            if(pIO->input.x_stride == 1 && pIO->output.x_stride == 2)
                            {
                               for (y = 0; y < pIO->input.height; y++)
                               {
                                   DVP_U08 *tmpIn  = (DVP_U08 *)DVP_Image_Addressing(&pIO->input, 0, y, p);
                                   DVP_U16 *tmpOut = (DVP_U16 *)DVP_Image_Addressing(&pIO->output, 0, y, p);
                                   if(kernel == DVP_KN_XSTRIDE_CONVERT)
                                       for(j = 0; j < pIO->input.width; j++)
                                       {
                                           tmpOut[j] = tmpIn[j];
                                       }
                                   else
                                       for(j = 0; j < pIO->input.width; j++)
                                       {
                                           tmpOut[j] = (DVP_U16)(tmpIn[j]<<8);
                                       }
                               }
                            }
                            else if(pIO->input.x_stride == 2 && pIO->output.x_stride == 1)
                            {
                               for (y = 0; y < pIO->input.height; y++)
                               {
                                   DVP_U16 *tmpIn  = (DVP_U16 *)DVP_Image_Addressing(&pIO->input, 0, y, p);
                                   DVP_U08 *tmpOut = (DVP_U08 *)DVP_Image_Addressing(&pIO->output, 0, y, p);
                                   if(kernel == DVP_KN_XSTRIDE_CONVERT)
                                       for(j = 0; j < pIO->input.width; j++)
                                       {
                                           tmpOut[j] = (DVP_U08)(tmpIn[j]);
                                       }
                                   else
                                       for(j = 0; j < pIO->input.width; j++)
                                       {
                                           tmpOut[j] = (DVP_U08)(tmpIn[j] >> 8);
                                       }
                               }
                            }
                            else
                            {
                                processed--;
                                pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                                break;
                            }
                        }
                    }
                    else
                    {
                        processed--;
                        pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                    }
                    break;
                }
                case DVP_KN_IMAGE_DEBUG:
                {
                    DVP_ImageDebug_t *pImgdbg = dvp_knode_to(&pSubNodes[n], DVP_ImageDebug_t);
                    DVP_Image_t *pImage = &pImgdbg->image;

#if defined(DVP_USE_FS)
                    if (pImgdbg->fp == NULL)
                    {
                        PYUV_GetFilename(pImgdbg->fullname, pImgdbg->path, pImgdbg->name, pImage->width, pImage->height, 1, pImage->color);
                        pImgdbg->fp = fopen(pImgdbg->fullname, "wb+");
                    }
                    if (pImgdbg->fp)
                    {
                        DVP_U32 y,p,len,ydiv,n = 0;
                        DVP_U08 *ptr = NULL;

                        for (p = 0; p < pImage->planes; p++)
                        {
                            ydiv = DVP_Image_HeightDiv(pImage, p);
                            len = DVP_Image_PatchLineSize(pImage, p);
                            for (y = 0; y < pImage->height/ydiv; y++)
                            {
                                ptr = DVP_Image_PatchAddressing(pImage, 0, y*ydiv, p);
                                n += (uint32_t)fwrite(ptr, 1, len, pImgdbg->fp);
                            }
                            fflush(pImgdbg->fp);
                        }
                        DVP_PRINT(DVP_ZONE_KGM, "Wrote %u bytes to %s\n", n, pImgdbg->fullname);
                    }
#endif // DVP_USE_FS
                    break;
                }
                case DVP_KN_BUFFER_DEBUG:
                {
                    DVP_U32 n = 0;
                    DVP_BufferDebug_t *pBufdbg = dvp_knode_to(&pSubNodes[n], DVP_BufferDebug_t);
                    DVP_Buffer_t *pBuffer = &pBufdbg->buffer;
#if defined(DVP_USE_FS)
                    if (pBufdbg->fp == NULL)
                    {
                        pBufdbg->fp = fopen(pBufdbg->fullname, "wb+");
                    }
                    if (pBufdbg->fp)
                    {
                        n += (uint32_t)fwrite(pBuffer->pData, 1, pBuffer->numBytes, pBufdbg->fp);
                        fflush(pBufdbg->fp);
                    }
#endif
                    break;
                }
                case DVP_KN_GAMMA:
                {
                    DVP_Gamma_t *pGamma = dvp_knode_to(&pSubNodes[n], DVP_Gamma_t);
                    gamma_apply(&pGamma->input, &pGamma->output, pGamma->gammaLut);
                    break;
                }
#if defined(DVP_USE_YUV) && defined(DVP_OPTIMIZED_KERNELS)
                case DVP_KN_OPT_UYVY_TO_RGBp_LUMA_YUV444p:
                {
                    DVP_Image_t *pUYVY    = &dvp_knode_to(&pSubNodes[n-2], DVP_Transform_t)->input;
                    DVP_Image_t *pRGB     = &dvp_knode_to(&pSubNodes[n-2], DVP_Transform_t)->output;
                    DVP_Image_t *pLuma    = &dvp_knode_to(&pSubNodes[n-1], DVP_Transform_t)->output;
                    DVP_Image_t *pYUV444p = &dvp_knode_to(&pSubNodes[n-0], DVP_Transform_t)->output;
                    /// @todo check to make sure all inputs are identical!
                    __uyvy_to_rgbp_luma_yuv444p_image(pUYVY->width,
                                                      pUYVY->height,
                                                      pUYVY->pData[0],
                                                      pUYVY->y_stride,
                                                      pRGB->pData[0],
                                                      pRGB->pData[1],
                                                      pRGB->pData[2],
                                                      pLuma->pData[0],
                                                      pYUV444p->pData[0],
                                                      pYUV444p->pData[1],
                                                      pYUV444p->pData[2]);
                    processed += 2;
                    break;
                }
                case DVP_KN_OPT_UYVY_TO_YUV444p_LUMA_RGBp:
                {
                    DVP_Image_t *pUYVY    = &dvp_knode_to(&pSubNodes[n-2], DVP_Transform_t)->input;
                    DVP_Image_t *pYUV444p = &dvp_knode_to(&pSubNodes[n-2], DVP_Transform_t)->output;
                    DVP_Image_t *pLuma    = &dvp_knode_to(&pSubNodes[n-1], DVP_Transform_t)->output;
                    DVP_Image_t *pRGB     = &dvp_knode_to(&pSubNodes[n-0], DVP_Transform_t)->output;
                    /// @todo check to make sure all inputs are identical!
                    __uyvy_to_rgbp_luma_yuv444p_image(pUYVY->width,
                                                      pUYVY->height,
                                                      pUYVY->pData[0],
                                                      pUYVY->y_stride,
                                                      pRGB->pData[0],
                                                      pRGB->pData[1],
                                                      pRGB->pData[2],
                                                      pLuma->pData[0],
                                                      pYUV444p->pData[0],
                                                      pYUV444p->pData[1],
                                                      pYUV444p->pData[2]);
                    processed += 2;
                    break;
                }
#endif
#if defined(DVP_USE_VLIB)
                case DVP_KN_VLIB_MOG_S16:
                {
                    DVP_Mog_t *mog = dvp_knode_to(&pSubNodes[n], DVP_Mog_t);

                    VLIB_mixtureOfGaussiansS16((DVP_U08*)mog->image.pData[0],
                                         (DVP_S16*)mog->wts.pData[0],
                                         (DVP_S16*)mog->means.pData[0],
                                         (DVP_S16*)mog->vars.pData[0],
                                         (DVP_U08*)mog->compIndex.pData[0],
                                         (DVP_U08*)mog->scratch.pData[0],
                                         (U32*)mog->fgmask.pData[0],
                                         (S32)(mog->image.width * mog->image.height),
                                         (DVP_S16) mog->alpha,
                                         (DVP_S16) mog->rho,
                                         (DVP_S16) mog->delta,
                                         (DVP_S16) mog->T,
                                         (DVP_S16) mog->ini_wt,
                                         (DVP_S16) mog->ini_var);
                    break;
                }


                case DVP_KN_VLIB_MOG_S32:
                {
                    DVP_Mog_t *mog = dvp_knode_to(&pSubNodes[n], DVP_Mog_t);

                    VLIB_mixtureOfGaussiansS32((DVP_U08*)mog->image.pData[0],
                                         (DVP_S16*)mog->wts.pData[0],
                                         (S32*)mog->means.pData[0],
                                         (S32*)mog->vars.pData[0],
                                         (DVP_U08*)mog->compIndex.pData[0],
                                         (DVP_U08*)mog->scratch.pData[0],
                                         (U32*)mog->fgmask.pData[0],
                                         (S32)(mog->image.width * mog->image.height),
                                         (DVP_S16) mog->alpha,
                                         (S32) mog->rho,
                                         (S32) mog->delta,
                                         (DVP_S16) mog->T,
                                         (DVP_S16) mog->ini_wt,
                                         (S32) mog->ini_var);
                    break;
                }


                case DVP_KN_VLIB_EWR_MEAN_S16:
                {
                    DVP_EMean_t *pM = dvp_knode_to(&pSubNodes[n], DVP_EMean_t);
                    DVP_U32 y;
                    for (y = 0; y < pM->data.height; y++)
                    {
                        VLIB_updateEWRMeanS16((S16 *)&pM->mean.pData[0][y * pM->mean.y_stride],
                                              &pM->data.pData[0][y * pM->data.y_stride],
                                              (U32 *)&pM->mask.pData[0][y * pM->mask.y_stride],
                                              saturate_s16(pM->weight),
                                              pM->data.width);
                    }
                    break;
                }
                case DVP_KN_VLIB_EWR_VAR_S16:
                {
                    DVP_EMean_t *pM = dvp_knode_to(&pSubNodes[n], DVP_EMean_t);
                    DVP_U32 y;
                    for (y = 0; y < pM->data.height; y++)
                    {
                        VLIB_updateEWRVarianceS16((S16 *)&pM->var.pData[0][y * pM->var.y_stride],
                                                  (S16 *)&pM->mean.pData[0][y * pM->mean.y_stride],
                                                  &pM->data.pData[0][y * pM->data.y_stride],
                                                  (U32 *)&pM->mask.pData[0][y * pM->mask.y_stride],
                                                  saturate_s16(pM->weight),
                                                  pM->data.width);
                    }
                    break;
                }
                case DVP_KN_VLIB_EWR_MEAN_S32:
                {
                    DVP_EMean_t *pM = dvp_knode_to(&pSubNodes[n], DVP_EMean_t);
                    DVP_U32 y;
                    for (y = 0; y < pM->data.height; y++)
                    {
                        VLIB_updateEWRMeanS32((S32 *)&pM->mean.pData[0][y * pM->mean.y_stride],
                                              &pM->data.pData[0][y * pM->data.y_stride],
                                              (U32 *)&pM->mask.pData[0][y * pM->mask.y_stride],
                                              pM->weight,
                                              pM->data.width);
                    }break;
                }
                case DVP_KN_VLIB_EWR_VAR_S32:
                {
                    DVP_EMean_t *pM = dvp_knode_to(&pSubNodes[n], DVP_EMean_t);
                    DVP_U32 y;
                    for (y = 0; y < pM->data.height; y++)
                    {
                        VLIB_updateEWRVarianceS32((S32 *)&pM->var.pData[0][y * pM->var.y_stride],
                                                  (S32 *)&pM->mean.pData[0][y * pM->mean.y_stride],
                                                  &pM->data.pData[0][y * pM->data.y_stride],
                                                  (U32 *)&pM->mask.pData[0][y * pM->mask.y_stride],
                                                  pM->weight,
                                                  pM->data.width);
                    }
                    break;
                }
                case DVP_KN_VLIB_UWR_MEAN_S16:
                {
                    DVP_UMean_t *pM = dvp_knode_to(&pSubNodes[n], DVP_UMean_t);
                    DVP_U32 y;
                    for (y = 0; y < pM->new_data.height; y++)
                    {
                        VLIB_updateUWRMeanS16((S16 *)&pM->new_mean.pData[0][y * pM->new_mean.y_stride],
                                              (S16 *)&pM->old_mean.pData[0][y * pM->old_mean.y_stride],
                                              &pM->new_data.pData[0][y * pM->new_data.y_stride],
                                              &pM->old_data.pData[0][y * pM->old_data.y_stride],
                                              (U32 *)&pM->new_mask.pData[0][y * pM->new_mask.y_stride],
                                              (U32 *)&pM->old_mask.pData[0][y * pM->old_mask.y_stride],
                                              pM->new_data.width,
                                              pM->bufferLength);
                    }
                    break;
                }
                case DVP_KN_VLIB_UWR_VAR_S16:
                {
                    DVP_UMean_t *pM = dvp_knode_to(&pSubNodes[n], DVP_UMean_t);
                    DVP_U32 y;
                    for (y = 0; y < pM->new_data.height; y++)
                    {
                        VLIB_updateUWRVarianceS16((S16 *)&pM->new_var.pData[0][y * pM->new_var.y_stride],
                                                  (S16 *)&pM->old_var.pData[0][y * pM->old_var.y_stride],
                                                  (S16 *)&pM->new_mean.pData[0][y * pM->new_mean.y_stride],
                                                  (S16 *)&pM->old_mean.pData[0][y * pM->old_mean.y_stride],
                                                  &pM->new_data.pData[0][y * pM->new_data.y_stride],
                                                  (U32 *)&pM->new_mask.pData[0][y * pM->new_mask.y_stride],
                                                  (U32 *)&pM->old_mask.pData[0][y * pM->old_mask.y_stride],
                                                  pM->new_data.width,
                                                  pM->bufferLength);
                    }
                    break;
                }
                case DVP_KN_VLIB_SUB_BACK_S16:
                {
                    DVP_SubBackground_t *pSB = dvp_knode_to(&pSubNodes[n], DVP_SubBackground_t);

                    /*number of output 32 packed values will be (frameSize/32) */
                    VLIB_subtractBackgroundS16((U32 *)pSB->output32packed.pData[0],
                               (DVP_U08 *)pSB->newestData.pData[0],
                               (DVP_S16 *)pSB->currentMean.pData[0],
                               (DVP_S16 *)pSB->currentVar.pData[0],
                               (short)pSB->thresholdGlobal,
                               (short)pSB->thresholdFactor,
                               (U32)(pSB->newestData.width * pSB->newestData.height ) );
                    break;
                }
                case DVP_KN_VLIB_SUB_BACK_S32:
                {
                    DVP_SubBackground_t *pSB = dvp_knode_to(&pSubNodes[n], DVP_SubBackground_t);

                    VLIB_subtractBackgroundS32((U32 *)pSB->output32packed.pData[0],
                               (DVP_U08 *)pSB->newestData.pData[0],
                               (S32 *)pSB->currentMean.pData[0],
                               (S32 *)pSB->currentVar.pData[0],
                               (S32)pSB->thresholdGlobal,
                               (S32)pSB->thresholdFactor,
                               (U32)(pSB->newestData.width * pSB->newestData.height ) );
                    break;
                }
                case DVP_KN_VLIB_EXTRACT_BACK_8_16:
                {
                    DVP_Transform_t *pEb = (DVP_Transform_t *)dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    DVP_U32 y = 0;
                    DVP_U08 *pBack = pEb->input.pData[0];
                    DVP_U08 *Out = pEb->output.pData[0];

                    for (y = 0; y < pEb->input.height; y++)
                    {
                        VLIB_extract8bitBackgroundS16((DVP_S16 *)&pBack[y * pEb->input.y_stride],
                                              &Out[y * pEb->output.y_stride],
                                              pEb->input.width);
                    }
                    break;
                }
                case DVP_KN_VLIB_PACK_MASK_32:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    DVP_U32 y;
                    for (y = 0; y < pT->input.height; y++)
                    {
                        VLIB_packMask32(&pT->input.pData[0][y * pT->input.y_stride],
                                        (U32 *)&pT->output.pData[0][y * pT->output.y_stride],
                                        pT->input.width);
                    }
                    break;
                }
                case DVP_KN_VLIB_UNPACK_MASK_32:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    DVP_U32 y;
                    for (y = 0; y < pT->input.height; y++)
                    {
                        VLIB_unpackMask32((U32 *)&pT->input.pData[0][y * pT->input.y_stride],
                                          &pT->output.pData[0][y * pT->output.y_stride],
                                          pT->input.width);
                    }
                    break;
                }
                case DVP_KN_DILATE_CROSS:
                case DVP_KN_DILATE_MASK:
                case DVP_KN_DILATE_SQUARE:
                case DVP_KN_ERODE_CROSS:
                case DVP_KN_ERODE_MASK:
                case DVP_KN_ERODE_SQUARE:
                case DVP_KN_VLIB_DILATE_CROSS:
                case DVP_KN_VLIB_DILATE_MASK:
                case DVP_KN_VLIB_DILATE_SQUARE:
                case DVP_KN_VLIB_ERODE_CROSS:
                case DVP_KN_VLIB_ERODE_MASK:
                case DVP_KN_VLIB_ERODE_SQUARE:
                case DVP_KN_VLIB_ERODE_SINGLEPIXEL:
                {
                    DVP_Morphology_t *pD = dvp_knode_to(&pSubNodes[n], DVP_Morphology_t);
                    DVP_U32 y, jv;
                    DVP_U32 *pPacked32in, *pPacked32out;
                    DVP_U08 *tmp;
                    DVP_U32 packed32Line;
                    size_t packed32Size;

                    packed32Line = pD->input.width / 32;                               // How many 32bit words per line
                    packed32Size = pD->input.height * packed32Line * sizeof(DVP_U32);  // How many byptes per image

#if defined(ANDROID)
                    pPacked32in  = (DVP_U32 *) memalign(64, packed32Size);
                    pPacked32out = (DVP_U32 *) memalign(64, packed32Size);
#elif defined(__QNX__) || defined(LINUX)
                    if (0 != posix_memalign((void *)&pPacked32in, 64, packed32Size))
                        break;
                    if (0 != posix_memalign((void *)&pPacked32out, 64, packed32Size))
                        break;
#else
                    pPacked32in  = (DVP_U32 *)calloc(1, packed32Size);
                    pPacked32out = (DVP_U32 *)calloc(1, packed32Size);
#endif
                    tmp = pD->input.pData[0];
                    for(jv=0; jv<pD->input.height; jv++)
                    {
                        VLIB_packMask32(&tmp[jv*pD->input.y_stride], &pPacked32in[jv*packed32Line], pD->input.width); // Assuming 1 byte per input pixel
                    }
#if 0
                    memcpy(pPacked32out, pPacked32in, packed32Size); // For testing the packing/unpacking functions
#else
                    switch(kernel)
                    {
                        case DVP_KN_DILATE_CROSS:
                        case DVP_KN_VLIB_DILATE_CROSS:
                            for (y = 0; y < (pD->input.height - 2); y++)
                            {
                                VLIB_dilate_bin_cross((U08 *)&pPacked32in[y * packed32Line],
                                                      (U08 *)&pPacked32out[y * packed32Line],
                                                      pD->input.width,
                                                      pD->input.width);
                            }
                            break;
                        case DVP_KN_DILATE_MASK:
                        case DVP_KN_VLIB_DILATE_MASK:
                            for (y = 0; y < (pD->input.height - 2); y++)
                            {
                                VLIB_dilate_bin_mask((U08 *)&pPacked32in[y * packed32Line],
                                                     (U08 *)&pPacked32out[y * packed32Line],
                                                     (S08 *)pD->mask.pData[0],
                                                     pD->input.width,
                                                     pD->input.width);
                            }
                            break;
                        case DVP_KN_DILATE_SQUARE:
                        case DVP_KN_VLIB_DILATE_SQUARE:
                            for (y = 0; y < (pD->input.height - 2); y++)
                            {
                                VLIB_dilate_bin_square((U08 *)&pPacked32in[y * packed32Line],
                                                       (U08 *)&pPacked32out[y * packed32Line],
                                                       pD->input.width,
                                                       pD->input.width);
                            }
                            break;
                        case DVP_KN_ERODE_CROSS:
                        case DVP_KN_VLIB_ERODE_CROSS:
                            for (y = 0; y < (pD->input.height - 2); y++)
                            {
                                VLIB_erode_bin_cross((U08 *)&pPacked32in[y * packed32Line],
                                                     (U08 *)&pPacked32out[y * packed32Line],
                                                     pD->input.width,
                                                     pD->input.width);
                            }
                            break;
                        case DVP_KN_ERODE_MASK:
                        case DVP_KN_VLIB_ERODE_MASK:
                            for (y = 0; y < (pD->input.height - 2); y++)
                            {
                                VLIB_erode_bin_mask((U08 *)&pPacked32in[y * packed32Line],
                                                    (U08 *)&pPacked32out[y * packed32Line],
                                                    (S08 *)pD->mask.pData[0],
                                                    pD->input.width,
                                                    pD->input.width);
                            }
                            break;
                        case DVP_KN_ERODE_SQUARE:
                        case DVP_KN_VLIB_ERODE_SQUARE:
                            for (y = 0; y < (pD->input.height - 2); y++)
                            {
                                VLIB_erode_bin_square((U08 *)&pPacked32in[y * packed32Line],
                                                      (U08 *)&pPacked32out[y * packed32Line],
                                                      pD->input.width,
                                                      pD->input.width);
                            }
                            break;
                        case DVP_KN_VLIB_ERODE_SINGLEPIXEL:
                            for (y = 0; y < (pD->input.height - 2); y++)
                            {
                                VLIB_erode_bin_singlePixel((U08 *)&pPacked32in[y * packed32Line],
                                                           (U08 *)&pPacked32out[y * packed32Line],
                                                           pD->input.width,
                                                           pD->input.width);
                            }
                            break;
                        default:
                            break;
                    }
#endif
                    tmp = pD->output.pData[0];
                    for(jv=0; jv<pD->output.height; jv++)
                    {
                        VLIB_unpackMask32(&pPacked32out[jv*packed32Line], &tmp[jv*pD->output.y_stride], pD->output.width); // Assuming 1 byte per input pixel
                    }
                    free(pPacked32in);
                    free(pPacked32out);
                    break;
                }
                case DVP_KN_VLIB_CCL:
                {
                    DVP_CCL_t *pC = dvp_knode_to(&pSubNodes[n], DVP_CCL_t);
                    DVP_S32 scratchSize;
                    VLIB_STATUS status;
                    DVP_S32 numCCs;
                    DVP_PTR pScratch;
                    DVP_U32 *pPacked32;
                    DVP_U32 packed32Line;
                    DVP_U32 packed32Size;
                    VLIB_CCHandle * handle;
                    DVP_U32 jv;
                    DVP_U08 *tmp = NULL;
                    VLIB_calcConnectedComponentsMaxBufferSize(pC->input.width,
                                                              pC->input.height,
                                                              pC->minBlobAreaInPixels,
                                                              &scratchSize);

                    /* Set-up Memory Buffers */
                    handle = (VLIB_CCHandle *) malloc(VLIB_GetSizeOfCCHandle());
#if defined(ANDROID)
                    pScratch    = (DVP_PTR) memalign(64, scratchSize);
#elif defined(__QNX__) || defined(LINUX)
                    if (0 != posix_memalign((void *)&pScratch, 64, scratchSize))
                        break;
#else
                    pScratch    = (DVP_PTR)calloc(1, scratchSize);
#endif
                    status = VLIB_initConnectedComponentsList(handle, pScratch, scratchSize);
                    if (status != VLIB_NO_ERROR)
                    {
                        DVP_PRINT(DVP_ZONE_ERROR, "VLIB_initConnectedComponentsList() failed with error code %d\n", status);
                        pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                        processed--;
                        break;
                    }

                    /* Pack data */
                    packed32Line = pC->input.width / 32;                               // How many 32bit words per line
                    packed32Size = pC->input.height * packed32Line * sizeof(DVP_U32);  // How many byptes per image
#if defined(ANDROID)
                    pPacked32    = (DVP_U32 *) memalign(64, packed32Size);
#elif defined(__QNX__) || defined(LINUX)
                    if (0 != posix_memalign((void *)&pPacked32, 64, packed32Size))
                        break;
#else
                    pPacked32    = (DVP_U32 *) calloc(1, packed32Size);
#endif
                    tmp = pC->input.pData[0];
                    for(jv=0; jv<pC->input.height; jv++)
                    {
                        VLIB_packMask32(&tmp[jv*pC->input.y_stride], &pPacked32[jv*packed32Line], pC->input.width); // Assuming 1 byte per input pixel
                    }

                    status = VLIB_createConnectedComponentsList(handle,
                                                                pC->input.width,
                                                                pC->input.height,
                                                                (U32*)(&pPacked32[0]),
                                                                pC->minBlobAreaInPixels,
                                                                pC->connected8Flag);

                    if (status == VLIB_WARNING_LOW_MEMORY)
                    {
                        DVP_PRINT(DVP_ZONE_KGM, "\nConnected Components low memory warning!!! Buffer within 1kbyte of allocation!!!\n");
                    }
                    else if (status != VLIB_NO_ERROR)
                    {
                        DVP_PRINT(DVP_ZONE_ERROR, "\nCreation of Connected Components failed!!!\n");
                        pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                        processed--;
                        break;
                    }

                    /* Retreive number of connected components */
                    VLIB_getNumCCs(handle, &numCCs);
                    pC->numCCs = numCCs;

                    /* Create Connected component Output Map */
                    memset(pC->output.pData[0], 0, pC->input.width*pC->input.height);   // Clearing output since function only writes to blobs.
                    status = VLIB_createCCMap8Bit(handle,
                                                  pC->output.pData[0],
                                                  pC->input.width,
                                                  pC->input.height);

                    if (status != 0)
                    {
                        DVP_PRINT(DVP_ZONE_ERROR, "Creation of 8-bit Connected Components Map failed!!!\n");
                        pSubNodes[n].header.error = DVP_ERROR_NO_MEMORY;
                        processed--;
                        break;
                    }
                    free(pPacked32);
                    free(pScratch);
                    free(handle);
                    break;
                }
                case DVP_KN_CANNY_2D_GRADIENT:
                case DVP_KN_VLIB_CANNY_2D_GRADIENT:
                {
                    DVP_Canny2dGradient_t *pG = dvp_knode_to(&pSubNodes[n], DVP_Canny2dGradient_t);
                    VLIB_xyGradientsAndMagnitude(pG->input.pData[0],
                                                 (S16 *)pG->outGradX.pData[0],
                                                 (S16 *)pG->outGradY.pData[0],
                                                 (S16 *)pG->outMag.pData[0],
                                                 (U16)pG->input.width,
                                                 (U16)pG->input.height);
                    break;
                }
                case DVP_KN_CANNY_NONMAX_SUPPRESSION:  // Assumes 1D buffer
                case DVP_KN_VLIB_CANNY_NONMAX_SUPPRESSION:
                {
                    DVP_CannyNonMaxSuppression_t *cnonmax = dvp_knode_to(&pSubNodes[n], DVP_CannyNonMaxSuppression_t);

                    VLIB_nonMaximumSuppressionCanny((S16 *)cnonmax->inMag.pData[0],
                                                (S16 *)cnonmax->inGradX.pData[0],
                                                (S16 *)cnonmax->inGradY.pData[0],
                                                cnonmax->output.pData[0],
                                                (U16)cnonmax->inMag.width,
                                                (U16)cnonmax->inMag.y_stride/2,
                                                (U16)cnonmax->inMag.height);
                    break;
                }
                case DVP_KN_CANNY_HYST_THRESHHOLD:
                case DVP_KN_VLIB_CANNY_HYST_THRESHHOLD:
                {
                    DVP_CannyHystThresholding_t *dthresh = dvp_knode_to(&pSubNodes[n], DVP_CannyHystThresholding_t);
                    VLIB_doublethresholding((DVP_S16 *)dthresh->inMag.pData[0],
                                            dthresh->inEdgeMap.pData[0],
                                            (DVP_U32 *)dthresh->output.pData[0],
                                            (DVP_S32 *)&dthresh->numEdges,
                                            (DVP_U16)dthresh->inMag.width,
                                            (DVP_U16)(dthresh->inMag.y_stride/dthresh->inMag.x_stride),
                                            (DVP_U16)dthresh->inMag.height,
                                            dthresh->loThresh,
                                            dthresh->hiThresh,
                                            dthresh->numEdges);
                    DVP_PRINT(DVP_ZONE_KGM, "There were %u strong edges found!\n", dthresh->numEdges);
                    break;
                }
                case DVP_KN_VLIB_IMAGE_PYRAMID_8:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    VLIB_imagePyramid8(pT->input.pData[0],
                                       (U16)pT->input.width,
                                       (U16)pT->input.height,
                                       pT->output.pData[0]);
                    break;
                }
                case DVP_KN_VLIB_IMAGE_PYRAMID_16:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    VLIB_imagePyramid16((U16 *)pT->input.pData[0],
                                        (U16)pT->input.width,
                                        (U16)pT->input.height,
                                        (U16 *)pT->output.pData[0]);
                    break;
                }
                case DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_8:
                {
                    DVP_Gradient_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Gradient_t);

                    for(i=0; i<pT->input.height/2; i+=1)
                    {
                        VLIB_gauss5x5PyramidKernel_8(&pT->input.pData[0][2*i*pT->input.y_stride],
                                                     (DVP_U16 *)pT->scratch.pData[0],
                                                     pT->input.width,
                                                     pT->input.width,
                                                     1,
                                                     &pT->output.pData[0][i*pT->output.y_stride]);
                    }
                    break;
                }
                case DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_16:
                {
                    DVP_Gradient_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Gradient_t);
                    for(i=0; i<pT->input.height/2; i+=1)
                    {
                        VLIB_gauss5x5PyramidKernel_16((DVP_U16 *)&pT->input.pData[0][2*i*pT->input.y_stride],
                                                      (DVP_U32 *)pT->scratch.pData[0],
                                                      pT->input.width,
                                                      pT->input.width,
                                                      1,
                                                      (DVP_U16 *)&pT->output.pData[0][i*pT->output.y_stride]);
                    }
                    break;
                }
                case DVP_KN_VLIB_GRADIENT_H5x5_PYRAMID_8:
                {
                    DVP_Gradient_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Gradient_t);
                    VLIB_gradientH5x5PyramidKernel_8(pT->input.pData[0],
                                                     (S16 *)pT->scratch.pData[0],
                                                     pT->input.width,
                                                     pT->input.y_stride/pT->input.x_stride,
                                                     pT->input.height,
                                                     pT->output.pData[0]);
                    break;
                }
                case DVP_KN_VLIB_GRADIENT_V5x5_PYRAMID_8:
                {
                    DVP_Gradient_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Gradient_t);
                    VLIB_gradientV5x5PyramidKernel_8(pT->input.pData[0],
                                                     (S16 *)pT->scratch.pData[0],
                                                     pT->input.width,
                                                     pT->input.y_stride/pT->input.x_stride,
                                                     pT->input.height,
                                                     pT->output.pData[0]);
                    break;
                }
                case DVP_KN_VLIB_HARRIS_SCORE_7x7:
                {
                    DVP_HarrisDSP_t *pH = dvp_knode_to(&pSubNodes[n], DVP_HarrisDSP_t);
                    VLIB_harrisScore_7x7((DVP_S16 *)pH->inGradX.pData[0],
                                         (DVP_S16 *)pH->inGradY.pData[0],
                                         pH->inGradX.width,
                                         pH->inGradX.height,
                                         (DVP_S16 *)pH->harrisScore.pData[0],
                                         pH->k,
                                         pH->scratch.pData);
                    break;
                }
                case DVP_KN_VLIB_HARRIS_SCORE_7x7_U32:
                {
                    DVP_HarrisDSP_t *pH = dvp_knode_to(&pSubNodes[n], DVP_HarrisDSP_t);
                    VLIB_harrisScore_U32_7x7((DVP_S16 *)pH->inGradX.pData[0],
                                             (DVP_S16 *)pH->inGradY.pData[0],
                                             pH->inGradX.width,
                                             pH->inGradX.height,
                                             (DVP_U32 *)pH->harrisScore.pData[0],
                                             pH->k,
                                             pH->scratch.pData);
                    break;
                }
                case DVP_KN_VLIB_TRACK_FEATURES_LUCAS_7x7:
                {
                    DVP_TrackFeatures_t *pTF = dvp_knode_to(&pSubNodes[n], DVP_TrackFeatures_t);
                    VLIB_trackFeaturesLucasKanade_7x7(pTF->im1.pData[0],
                                                      pTF->im2.pData[0],
                                                      (DVP_S16 *)pTF->gradx.pData[0],
                                                      (DVP_S16 *)pTF->grady.pData[0],
                                                      pTF->im1.width,
                                                      pTF->im1.height,
                                                      pTF->nfeatures,
                                                      (DVP_S16 *)pTF->x.pData,
                                                      (DVP_S16 *)pTF->y.pData,
                                                      (DVP_S16 *)pTF->outx.pData,
                                                      (DVP_S16 *)pTF->outy.pData,
                                                      pTF->max_iters,
                                                      pTF->scratch.pData);
                    break;
                }
                case DVP_KN_IIR_HORZ:
                case DVP_KN_VLIB_IIR_HORZ:
                {
                    DVP_IIR_t *pT = dvp_knode_to(&pSubNodes[n], DVP_IIR_t);
                    DVP_U32 y;
                    if((int)pT->input.width == pT->input.y_stride)
                    {
                        VLIB_recursiveFilterHoriz1stOrder(pT->output.pData[0],
                                                          pT->input.pData[0],
                                                          pT->input.width,
                                                          pT->input.height,
                                                          pT->weight,
                                                          pT->bounds[0].pData,
                                                          pT->bounds[1].pData,
                                                          pT->scratch.pData[0]);
                    }else
                    {
                        DVP_U08 *tmpBuf = (DVP_U08 *)malloc(pT->input.width * 8);
                        DVP_U32 i;
                        for(y=0; y<pT->input.height; y+=8)
                        {
                            for(i=0; i<8; i++)
                                memcpy(&tmpBuf[i*pT->input.width], &pT->input.pData[0][(y+i)*pT->input.y_stride], pT->input.width);

                            VLIB_recursiveFilterHoriz1stOrder(&pT->output.pData[0][y*pT->output.y_stride],
                                                              tmpBuf,
                                                              pT->input.width,
                                                              8,
                                                              pT->weight,
                                                              pT->bounds[0].pData,
                                                              pT->bounds[1].pData,
                                                              pT->scratch.pData[0]);
                        }
                        free(tmpBuf);
                    }
                    break;
                }
                case DVP_KN_VLIB_IIR_HORZ_16:
                {
                    DVP_IIR_t *pT = dvp_knode_to(&pSubNodes[n], DVP_IIR_t);
                    DVP_U32 y;
                    if((int)(2*pT->input.width) == pT->input.y_stride)
                    {
                        VLIB_recursiveFilterHoriz1stOrderS16((S16 *)pT->output.pData[0],
                                                          (S16 *)pT->input.pData[0],
                                                          pT->input.width,
                                                          pT->input.height,
                                                          pT->weight,
                                                          (S16 *)pT->bounds[0].pData,
                                                          (S16 *)pT->bounds[1].pData,
                                                          (S16 *)pT->scratch.pData[0]);
                    }else
                    {
                        DVP_U08 *tmpBuf = (DVP_U08 *)malloc(pT->input.width*8*sizeof(S16));
                        DVP_U32 i;
                        for(y=0; y<pT->input.height; y+=8)
                        {
                            for(i=0; i<8; i++)
                                memcpy(&tmpBuf[i*pT->input.width*sizeof(S16)], &pT->input.pData[0][(y+i)*pT->input.y_stride], pT->input.width*sizeof(S16));

                            VLIB_recursiveFilterHoriz1stOrderS16((S16 *)&pT->output.pData[0][y*pT->output.y_stride],
                                                              (S16 *)tmpBuf,
                                                              pT->input.width,
                                                              8,
                                                              pT->weight,
                                                              (S16 *)pT->bounds[0].pData,
                                                              (S16 *)pT->bounds[1].pData,
                                                              (S16 *)pT->scratch.pData[0]);
                        }
                        free(tmpBuf);
                    }
                    break;
                }
                case DVP_KN_IIR_VERT:
                case DVP_KN_VLIB_IIR_VERT:
                {
                    DVP_IIR_t *pT = dvp_knode_to(&pSubNodes[n], DVP_IIR_t);
                    if((int)pT->input.width == pT->input.y_stride)
                    {
                        VLIB_recursiveFilterVert1stOrder(pT->output.pData[0],
                                                         pT->input.pData[0],
                                                         pT->input.width,
                                                         pT->input.height,
                                                         pT->weight,
                                                         pT->bounds[0].pData,
                                                         pT->bounds[1].pData,
                                                         pT->scratch.pData[0]);
                    }else
                    {
                        DVP_U08 *tmpBufIn = (DVP_U08 *)malloc(pT->input.height * pT->input.width);
                        DVP_U32 i;
                        for(i=0; i<pT->input.height; i++)
                            memcpy(&tmpBufIn[i*pT->input.width], pT->input.pData[0] + (i*pT->input.y_stride), pT->input.width);

                        VLIB_recursiveFilterVert1stOrder(pT->output.pData[0],
                                                          tmpBufIn,
                                                          pT->input.width,
                                                          pT->input.height,
                                                          pT->weight,
                                                          pT->bounds[0].pData,
                                                          pT->bounds[1].pData,
                                                          pT->scratch.pData[0]);
                        free(tmpBufIn);
                    }
                    break;
                }
                case DVP_KN_VLIB_IIR_VERT_16:
                {
                    DVP_IIR_t *pT = dvp_knode_to(&pSubNodes[n], DVP_IIR_t);
                    if((int)(2*pT->input.width) == pT->input.y_stride)
                    {
                        VLIB_recursiveFilterVert1stOrderS16((S16 *)pT->output.pData[0],
                                                         (S16 *)pT->input.pData[0],
                                                         pT->input.width,
                                                         pT->input.height,
                                                         pT->weight,
                                                         (S16 *)pT->bounds[0].pData,
                                                         (S16 *)pT->bounds[1].pData,
                                                         (S16 *)pT->scratch.pData[0]);
                    }else
                    {
                        DVP_U08 *tmpBufIn = (DVP_U08 *)malloc(pT->input.height * pT->input.width * sizeof(DVP_U16));
                        DVP_U32 i;
                        for(i=0; i<pT->input.height; i++)
                            memcpy(&tmpBufIn[i*pT->input.width*sizeof(DVP_U16)], pT->input.pData[0] + (i*pT->input.y_stride), pT->input.width* sizeof(DVP_U16));

                        VLIB_recursiveFilterVert1stOrderS16((S16 *)pT->output.pData[0],
                                                          (S16 *)tmpBufIn,
                                                          pT->input.width,
                                                          pT->input.height,
                                                          pT->weight,
                                                          (S16 *)pT->bounds[0].pData,
                                                          (S16 *)pT->bounds[1].pData,
                                                          (S16 *)pT->scratch.pData[0]);
                        free(tmpBufIn);
                    }
                    break;
                }
                case DVP_KN_INTEGRAL_IMAGE_8:
                case DVP_KN_VLIB_INTEGRAL_IMAGE_8:
                {
                    // param check (U32 width of output buffer).
                    // no stride

                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    for(i=0; i<pT->input.height-1; i++)
                    {
                        VLIB_integralImage8(&pT->input.pData[0][i*pT->input.y_stride],
                                            (U16)pT->input.width,
                                            1,
                                            (U32 *)&pT->output.pData[0][(pT->output.height-1)*pT->output.y_stride],
                                            (U32 *)&pT->output.pData[0][i*pT->output.y_stride]);
                    }
                    break;
                }
                case DVP_KN_VLIB_INTEGRAL_IMAGE_16:
                {
                    // param check (U32 width of output buffer).
                    // no stride

                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    VLIB_integralImage16((U16 *)pT->input.pData[0],
                                         pT->input.width,
                                         pT->input.height,
                                         (U32 *)&pT->output.pData[0][(pT->output.height-1)*pT->output.y_stride],
                                         (U32 *)pT->output.pData[0]);
                    break;
                }
                case DVP_KN_VLIB_HOUGH_LINE_FROM_LIST:
                {
                    DVP_HoughLine_t *pHl = dvp_knode_to(&pSubNodes[n], DVP_HoughLine_t);

                    VLIB_houghLineFromList((DVP_U16 *)pHl->pEdgeMapList.pData,
                                 (DVP_U16 *)pHl->pOutHoughSpace.pData[0],
                                 (DVP_U16)pHl->outBlkWidth,
                                 (DVP_U16)pHl->outBlkHeight,
                                 (U32)pHl->listSize,
                                 (DVP_U16)pHl->thetaRange,
                                 (DVP_U16)pHl->rhoMaxLength,
                                 (DVP_S16 *)pHl->pSIN.pData,
                                 (DVP_S16 *)pHl->pCOS.pData,
                                 (DVP_U16 *)pHl->ping.pData,
                                 (DVP_U16 *)pHl->pong.pData,
                                 (DVP_U16 *)pHl->pang.pData,
                                 (DVP_U16 *)pHl->peng.pData);
                    break;
                }
                case DVP_KN_VLIB_HYST_THRESHOLD:
                {
                    DVP_CannyHystThresholding_t *dthresh = dvp_knode_to(&pSubNodes[n], DVP_CannyHystThresholding_t);

                    VLIB_hysteresisThresholding((DVP_S16 *)dthresh->inMag.pData[0],
                                          (DVP_U08 *)dthresh->inEdgeMap.pData[0],
                                          (DVP_U08 *)dthresh->output.pData[0], //used as SCRATCH
                                          (DVP_U16)dthresh->inMag.height,
                                          (DVP_U16)dthresh->inMag.width,
                                           dthresh->loThresh,
                                           dthresh->hiThresh);
                    break;
                }
                case DVP_KN_NONMAXSUPPRESS_3x3_S16:
                case DVP_KN_VLIB_NONMAXSUPPRESS_3x3_S16:
                {
                    DVP_Threshold_t *pM = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    VLIB_nonMaxSuppress_3x3_S16((S16 *)pM->input.pData[0],
                                               (U16) pM->input.width,
                                                (U16)pM->input.height,
                                                pM->thresh,
                                                (U08 *)(pM->output.pData[0]+pM->output.width*2)); // In order to match simcop, output starts masksize-1 lines in
                    break;
                }
                case DVP_KN_NONMAXSUPPRESS_5x5_S16:
                case DVP_KN_VLIB_NONMAXSUPPRESS_5x5_S16:
                {
                    DVP_Threshold_t *pM = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    VLIB_nonMaxSuppress_5x5_S16((S16 *)pM->input.pData[0],
                                                (U16)pM->input.width,
                                                (U16)pM->input.height,
                                                pM->thresh,
                                                (U08 *)(pM->output.pData[0]+pM->output.width*4)); // In order to match simcop, output starts masksize-1 lines in
                    break;
                }
                case DVP_KN_NONMAXSUPPRESS_7x7_S16:
                case DVP_KN_VLIB_NONMAXSUPPRESS_7x7_S16:
                {
                    DVP_Threshold_t *pM = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    VLIB_nonMaxSuppress_7x7_S16((S16 *)pM->input.pData[0],
                                                (U16)pM->input.width,
                                                (U16)pM->input.height,
                                                pM->thresh,
                                                (U08 *)(pM->output.pData[0]+pM->output.width*6)); // In order to match simcop, output starts masksize-1 lines in
                    break;
                }
                case DVP_KN_VLIB_NORMALFLOW_16:
                {
                    DVP_NormalFlow_t *pF = dvp_knode_to(&pSubNodes[n], DVP_NormalFlow_t);
                    VLIB_normalFlow_16((S16 *)pF->input.pData[0],
                                       (S16 *)pF->mag.pData[0],
                                       (S16 *)pF->x.pData[0],
                                       (S16 *)pF->y.pData[0],
                                       (S16 *)pF->lut.pData[0],
                                       pF->thresh,
                                       pF->input.width * pF->input.height,
                                       (S16 *)pF->u.pData[0],
                                       (S16 *)pF->v.pData[0]);
                    break;
                }
                /// @todo case DVP_KN_VLIB_KALMAN_2x4:
                /// @todo case DVP_KN_VLIB_KALMAN_4x6:
                case DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_16:
                {
                    DVP_Simplex_t *pSmx = dvp_knode_to(&pSubNodes[n], DVP_Simplex_t);
                    void *addtlArgsPtr;
                    addtlArgsPtr = (void*)&(pSmx->addtlArgs);

                    VLIB_simplex(rosenbrock32,
                                (DVP_S16 *)pSmx->start.pData,
                                (DVP_S16 *)pSmx->step.pData,
                                pSmx->N_val,
                                pSmx->N_INV,
                                pSmx->MaxIteration,
                                pSmx->EPSILON,
                                (DVP_S16 *)pSmx->intBuf[0].pData,
                                (DVP_S32 *)pSmx->intBuf[1].pData,
                                (DVP_S16 *)pSmx->intBuf[2].pData,
                                (DVP_S16 *)pSmx->intBuf[3].pData,
                                (DVP_S16 *)pSmx->intBuf[4].pData,
                                (DVP_S16 *)pSmx->intBuf[5].pData,
                                addtlArgsPtr,
                                (DVP_S16 *)pSmx->stop.pData,
                                &(pSmx->minValue));
                    break;
                }
                case DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_3D:
                {
                    DVP_Simplex_t *pSmx = dvp_knode_to(&pSubNodes[n], DVP_Simplex_t);
                    void *addtlArgsPtr;

                    addtlArgsPtr = (void*)&(pSmx->addtlArgs);

                    VLIB_simplex_3D(rosenbrock32,
                                (DVP_S16 *)pSmx->start.pData,
                                (DVP_S16 *)pSmx->step.pData,
                                pSmx->MaxIteration,
                                pSmx->EPSILON,
                                (DVP_S16 *)pSmx->intBuf[0].pData,
                                (DVP_S32 *)pSmx->intBuf[1].pData,
                                (DVP_S16 *)pSmx->intBuf[2].pData,
                                (DVP_S16 *)pSmx->intBuf[3].pData,
                                (DVP_S16 *)pSmx->intBuf[4].pData,
                                (DVP_S16 *)pSmx->intBuf[5].pData,
                                addtlArgsPtr,
                                (DVP_S16 *)pSmx->stop.pData,
                                &(pSmx->minValue));
                    break;
                }
                /// @todo case DVP_KN_VLIB_LEGENDRE_MOMENTS:
                case DVP_KN_VLIB_INIT_HISTOGRAM_8:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                    VLIB_histogram_1D_Init_U8(pH->edges.pData,
                                              pH->numBins,
                                              pH->histArray.pData);
                    break;
                }
                case DVP_KN_VLIB_HISTOGRAM_8:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);

                    if(pSubNodes[n].header.resv[0] == 0)
                    {
                        VLIB_histogram_1D_Init_U8(pH->edges.pData,
                                                  pH->numBins,
                                                  pH->histArray.pData);
                        pSubNodes[n].header.resv[0] = 1;
                    }
                    if(pH->clearFlag)
                    {
                        memset(pH->h[0].pData, 0, pH->numBins*2);
                        memset(pH->h[1].pData, 0, pH->numBins*2);
                        memset(pH->h[2].pData, 0, pH->numBins*2);
                        memset(pH->hOut.pData, 0, pH->numBins*2);
                    }
                    VLIB_histogram_1D_U8(pH->input.pData[0],
                                         pH->input.numBytes,
                                         pH->numBins,
                                         pH->binWeight,
                                         pH->histArray.pData,
                                         (U16 *)pH->h[0].pData,
                                         (U16 *)pH->h[1].pData,
                                         (U16 *)pH->h[2].pData,
                                         (U16 *)pH->hOut.pData);
                    break;
                }
                case DVP_KN_VLIB_WEIGHTED_HISTOGRAM_8:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);

                    if(pSubNodes[n].header.resv[0] == 0)
                    {
                        VLIB_histogram_1D_Init_U8(pH->edges.pData,
                                                  pH->numBins,
                                                  pH->histArray.pData);
                        pSubNodes[n].header.resv[0] = 1;
                    }
                    if(pH->clearFlag)
                    {
                        memset(pH->h[0].pData, 0, pH->numBins*2);
                        memset(pH->h[1].pData, 0, pH->numBins*2);
                        memset(pH->h[2].pData, 0, pH->numBins*2);
                        memset(pH->hOut.pData, 0, pH->numBins*2);
                    }
                    VLIB_weightedHistogram_1D_U8(pH->input.pData[0],
                                                 pH->input.numBytes,
                                                 pH->numBins,
                                                 (U16 *)pH->binWeights.pData,
                                                 pH->histArray.pData,
                                                 (U16 *)pH->h[0].pData,
                                                 (U16 *)pH->h[1].pData,
                                                 (U16 *)pH->h[2].pData,
                                                 (U16 *)pH->hOut.pData);
                    break;
                }
                case DVP_KN_VLIB_INIT_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                    VLIB_histogram_1D_Init_U16((U16 *)pH->edges.pData,
                                               pH->numBins,
                                               (U16 *)pH->histArray.pData);
                    break;
                }
                case DVP_KN_VLIB_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);

                    if(pSubNodes[n].header.resv[0] == 0)
                    {
                        VLIB_histogram_1D_Init_U16((U16 *)pH->edges.pData,
                                                  pH->numBins,
                                                  (U16 *) pH->histArray.pData);
                        pSubNodes[n].header.resv[0] = 1;
                    }
                    if(pH->clearFlag)
                    {
                        memset(pH->h[0].pData,  0, pH->numBins*2);
                        memset(pH->hOut.pData, 0, pH->numBins*2);
                    }
                    VLIB_histogram_1D_U16((U16 *)pH->input.pData[0],
                                          pH->input.numBytes/2,
                                          pH->numBins,
                                          pH->binWeight,
                                          (U16 *)pH->histArray.pData,
                                          (U16 *)pH->h[0].pData,
                                          (U16 *)pH->hOut.pData);
                    break;
                }
                case DVP_KN_VLIB_WEIGHTED_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);

                    if(pSubNodes[n].header.resv[0] == 0)
                    {
                        VLIB_histogram_1D_Init_U16((U16 *)pH->edges.pData,
                                                  pH->numBins,
                                                  (U16 *) pH->histArray.pData);
                        pSubNodes[n].header.resv[0] = 1;
                    }
                    if(pH->clearFlag)
                    {
                        memset(pH->h[0].pData,  0, pH->numBins*2);
                        memset(pH->hOut.pData, 0, pH->numBins*2);
                    }
                    VLIB_weightedHistogram_1D_U16((U16 *)pH->input.pData[0],
                                                  pH->input.numBytes/2,
                                                  pH->numBins,
                                                  (U16 *)pH->binWeights.pData,
                                                  (U16 *)pH->histArray.pData,
                                                  (U16 *)pH->h[0].pData,
                                                  (U16 *)pH->hOut.pData);
                    break;
                }
                case DVP_KN_VLIB_HISTOGRAM_ND_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                    DVP_U32 totBins = 1;

                    for(i=0; i<pH->dimX; i++)
                       totBins *= pH->numBinsArray.pData[i];
                    if(pH->clearFlag)
                    {
                        memset(pH->hOut.pData, 0, totBins*2);
                    }
                    VLIB_histogram_nD_U16((DVP_U16 *)pH->input.pData[0],
                                           (pH->input.numBytes/2)/pH->dimX,
                                           pH->dimX,
                                           pH->binWeight,
                                           (DVP_U16 *)pH->numBinsArray.pData,
                                           (DVP_U16 *)pH->h[0].pData,
                                           (DVP_U16 *)pH->h[1].pData,
                                           (DVP_U16 *)pH->h[2].pData,
                                           (DVP_U16 *)pH->hOut.pData);
                    break;
                }
                case DVP_KN_VLIB_NONMAXSUPPRESS_U32:
                {
                    DVP_Nonmax_NxN_t *nonmax32 = dvp_knode_to(&pSubNodes[n], DVP_Nonmax_NxN_t);
                    VLIB_nonMaxSuppress_U32((DVP_U32 *)nonmax32->input.pData[0],
                                            (DVP_S32)nonmax32->input.width,
                                            (DVP_S32)nonmax32->input.height,
                                            (DVP_S32)nonmax32->p,
                                            (DVP_U32)nonmax32->threshold,
                                            (DVP_U32 *)nonmax32->scratch.pData,
                                            (DVP_S16 *)nonmax32->pixIndex.pData);
                    break;
                }

#if 0 // @todo Must test and enable these features
                case DVP_KN_VLIB_HISTOGRAM_ND_16:
                {
                    // @todo Must support more than 1 dimension!
                    DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                    VLIB_weightedHistogram_nD_U16((U16 *)pH->input.pData[0],
                                                  pH->input.numBytes,
                                                  1,
                                                  pH->numBins,
                                                  (U16 *)pH->binWeights.pData,
                                                  (U16 *)pH->histArray.pData,
                                                  (U16 *)pH->h[1].pData,
                                                  (U16 *)pH->h[2].pData,
                                                  (U16 *)pH->h[3].pData,
                                                  (U16 *)pH->h[0].pData);
                    break;
                }
#endif
                case DVP_KN_VLIB_BHATTACHAYA_DISTANCE:
                {
                    DVP_Distance_t *pD = dvp_knode_to(&pSubNodes[n], DVP_Distance_t);
                    DVP_U08* inX  = pD->X.pData[0];
                    DVP_U08* inY  = pD->Y.pData[0];

                    DVP_U32 y = 0;
                    DVP_U32 distance = 0;
                    for (y = 0; y < pD->X.height; y++)
                    {
                        distance = 0;
                        VLIB_bhattacharyaDistance_U32(
                            (U16 *)&inX[y*pD->X.y_stride],
                            (U16 *)&inY[y*pD->Y.y_stride],
                            pD->X.width,
                            &distance);
                        pD->distance += distance;
                    }

                    break;
                }
                case DVP_KN_VLIB_L1DISTANCE:
                {
                    DVP_U32 y = 0;
                    DVP_U32 distance;
                    DVP_Distance_t *pD = dvp_knode_to(&pSubNodes[n], DVP_Distance_t);
                    pD->distance = 0;
                    for (y = 0; y < pD->X.height; y++)
                    {
                        distance = 0;
                        VLIB_L1DistanceS16((S16 *)&pD->X.pData[0][y*pD->X.y_stride],
                                           (S16 *)&pD->Y.pData[0][y*pD->Y.y_stride],
                                           pD->X.width,
                                           &distance);
                        pD->distance += distance;
                    }
                    break;
                }
#endif
#if defined(DVP_USE_ORB)
                case DVP_KN_ORB:
                {
                    DVP_PRINT(DVP_ZONE_KGM, "Executing Kernel \"ARM\" ORB");
                    DVP_PRINT(DVP_ZONE_ALWAYS, "Running ORB on CPU");

                    CORBResult orbStatus = CORB_OK;
                    //Obtain the orbConfiguration data from Host side dvp struct
                    DVP_Orb_t * orbData = dvp_knode_to(&pSubNodes[n], DVP_Orb_t);
                    DVP_U32 imgWidth, imgHeight, x_stride, y_stride;
                    //local variables
                    CORBOutput orbOutput;
                    unsigned char * outImage[3]; //3 planes for output

                    if (orbInit == 0)
                    {
                        orbInput = (CORBInput  *)malloc(sizeof(CORBInput)*8);
                        if(orbInput == NULL)
                        {
                            DVP_PRINT(DVP_ZONE_ERROR, "ORB Input allocation failed. Exiting.");
                            break;
                        }
                    }

                    imgWidth = orbData->inputImage.width;
                    imgHeight = orbData->inputImage.height;
                    orbInput->image          = (unsigned char *)orbData->inputImage.pData[0];
                    orbInput->harrisImage    = (short *)        orbData->harrisImage.pData[0];
                    orbInput->integralImage  = (unsigned int *) orbData->integralImage.pData[0];
                    orbInput->orbPattern31   = (signed char *)  orbData->orbPattern31.pData;

                    orbOutput.features = (CORBFeature *) orbData->orbOutput.orbData.pData;
                    orbOutput.n_features = orbData->orbOutput.nfeatures;
                    //Copy plane pointers
                    outImage[0] = orbData->orbOutput.outImage.pData[0];
                    outImage[1] = orbData->orbOutput.outImage.pData[1];
                    outImage[2] = orbData->orbOutput.outImage.pData[2];
                    x_stride = orbData->orbOutput.outImage.x_stride;
                    y_stride = orbData->orbOutput.outImage.y_stride;
                    const CORBParameters orbParams = getCORBParameters(imgWidth, imgHeight, 35, 20, FAST_9, BOOL_TRUE, 1311, orbOutput.n_features);

                    if (orbInit == 0)
                    {
                        orbInit = 1;

                        //Returns the amount of required scratch memory
                        int orbScratchSize = getRequiredScratchSize(&orbParams, BOOL_FALSE, BOOL_TRUE);//adding flag for computing harris, integral image
                        DVP_PRINT(DVP_ZONE_KGM, "ORB Scratch size = %d bytes", orbScratchSize);
                        orbMem =  (DVP_U08 *)malloc(orbScratchSize);
                        DVP_PRINT(DVP_ZONE_KGM, "ORB Scratch allocated in external memory");

                        if(orbMem != NULL)
                        {
                            //Call ORB
                            orbStatus =  cORB_init(&orbParams, orbInput);
                        }
                        else
                        {
                            DVP_PRINT(DVP_ZONE_KGM, "ORB Scratch allocation failed. Exiting.");
                            break;
                        }
                    }

                    if ((orbMem != NULL) && (orbInput != NULL))
                    {
                        orbInput->scratchBuffer = (char*) orbMem;

                        if(orbStatus == CORB_OK)
                        {
                            orbStatus =  cORB_computeORB(&orbParams, orbInput, &orbOutput);
                        }
                        else
                        {
                            DVP_PRINT(DVP_ZONE_ERROR, "Error initializing ORB \n");
                        }

                        if(orbStatus == CORB_OK)
                        {
                            orbData->orbOutput.nfeatures = orbOutput.n_features;
                            orbStatus = cORB_showFeatures(&orbOutput, outImage, y_stride, x_stride, imgWidth, imgHeight);
                        }
                        else
                        {
                            DVP_PRINT(DVP_ZONE_ERROR, "Error running ORB \n");
                        }

                        if(orbStatus != CORB_OK)
                        {
                            DVP_PRINT(DVP_ZONE_ERROR, "Error displaying ORB features \n");
                        }
                    }
                    processed++;
                break;
                }
#endif//DVP_USE_ORB

#if defined(DVP_USE_YUV)
                //******************************************************
                // YUV ONLY CASES
                //******************************************************

                case DVP_KN_YUV_UYVY_HALF_SCALE:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    // the conversion call is based on the input to output formats
                    if (pT->input.color == FOURCC_UYVY &&
                        pT->output.color == FOURCC_UYVY &&
                        pT->input.width == (2*pT->output.width) &&
                        pT->input.height == (2*pT->output.height))
                    {
                        DVP_PrintImage(DVP_ZONE_KGM, &pT->input);
                        DVP_PrintImage(DVP_ZONE_KGM, &pT->output);
                        __uyvy_half_scale_image(pT->input.width,
                                                pT->input.height,
                                                pT->input.pData[0],
                                                pT->input.y_stride,
                                                pT->output.pData[0],
                                                pT->output.y_stride);
                    }
                    else
                        processed -=1;
                    break;
                }
                case DVP_KN_YUV_UYVY_QTR_SCALE:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    // the conversion call is based on the input to output formats
                    if (pT->input.color == FOURCC_UYVY &&
                        pT->output.color == FOURCC_UYVY &&
                        pT->input.width == (4*pT->output.width) &&
                        pT->input.height == (4*pT->output.height))
                    {
                        DVP_PrintImage(DVP_ZONE_KGM, &pT->input);
                        DVP_PrintImage(DVP_ZONE_KGM, &pT->output);
                        __uyvy_qtr_scale_image(pT->input.width,
                                               pT->input.height,
                                               pT->input.pData[0],
                                               pT->input.y_stride,
                                               pT->output.pData[0],
                                               pT->output.y_stride);
                    }
                    else
                        processed -=1;
                    break;
                }
                case DVP_KN_NV12_TO_YUV444p:
                case DVP_KN_YUV_NV12_TO_YU24_HALF_SCALE:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    //DVP_PrintImage(DVP_ZONE_KGM, &pT->input);
                    //DVP_PrintImage(DVP_ZONE_KGM, &pT->output);
                    if (pT->input.color == FOURCC_NV12 &&
                        pT->output.color == FOURCC_YV24)
                    {
                        __nv12_to_half_yuv444_image(pT->input.width,
                                                    pT->input.height,
                                                    pT->input.pData[0],
                                                    pT->input.pData[1],
                                                    pT->input.y_stride,
                                                    pT->output.pData[0],
                                                    pT->output.pData[2],
                                                    pT->output.pData[1],
                                                    pT->output.y_stride);
                    }
                    else if (pT->input.color == FOURCC_NV12 &&
                             pT->output.color == FOURCC_YU24)
                    {
                        __nv12_to_half_yuv444_image(pT->input.width,
                                                    pT->input.height,
                                                    pT->input.pData[0],
                                                    pT->input.pData[1],
                                                    pT->input.y_stride,
                                                    pT->output.pData[0],
                                                    pT->output.pData[1],
                                                    pT->output.pData[2],
                                                    pT->output.y_stride);
                    }
                    break;
                }
                case DVP_KN_YUV444p_TO_RGBp:
                case DVP_KN_YUV_YUV444_TO_RGBp:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == FOURCC_YU24 &&
                        pT->output.color == FOURCC_RGBP)
                    {
                        __yuv444_to_rgbp_image_bt601(pT->input.width,
                                                   pT->input.height,
                                                   pT->input.pData[0],
                                                   pT->input.pData[1],
                                                   pT->input.pData[2],
                                                   pT->input.y_stride,
                                                   pT->output.pData[0],
                                                   pT->output.pData[1],
                                                   pT->output.pData[2],
                                                   pT->output.y_stride);
                    }
                    else if (pT->input.color == FOURCC_YV24 &&
                             pT->output.color == FOURCC_RGBP)
                    {
                        __yuv444_to_rgbp_image_bt601(pT->input.width,
                                                   pT->input.height,
                                                   pT->input.pData[0],
                                                   pT->input.pData[2],
                                                   pT->input.pData[1],
                                                   pT->input.y_stride,
                                                   pT->output.pData[0],
                                                   pT->output.pData[1],
                                                   pT->output.pData[2],
                                                   pT->output.y_stride);
                    }
                    break;
                }
                case DVP_KN_YUV_Y800_ROTATE_CW_90:
                case DVP_KN_YUV_Y800_ROTATE_CCW_90:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == pT->output.color &&
                        pT->input.width == pT->output.height &&
                        pT->input.height == pT->output.width &&
                        pT->input.planes == pT->output.planes &&
                        (DVP_U32)pT->input.y_stride == pT->input.width &&
                        (DVP_U32)pT->output.y_stride == pT->output.width)
                    {
                        uint32_t p = 0;
                        for (p = 0; p < pT->input.planes; p++)
                        {
                            uint32_t div_x = 1;
                            uint32_t div_y = 1;
                            if (p > 0 )
                            {
                                if (pT->input.color == FOURCC_IYUV ||
                                    pT->input.color == FOURCC_YV12)
                                {
                                    div_x = 2;
                                    div_y = 2;
                                }
                                else if (pT->input.color == FOURCC_YU16 ||
                                         pT->input.color == FOURCC_YV16)
                                {
                                    div_x = 2;
                                }
                            }

                            if (kernel == DVP_KN_YUV_Y800_ROTATE_CW_90)
                                __planar_rotate_cw90(pT->input.width/div_x,
                                                     pT->input.height/div_y,
                                                     pT->input.pData[p],
                                                     pT->output.pData[p]);
                            else if (kernel == DVP_KN_YUV_Y800_ROTATE_CCW_90)
                                __planar_rotate_ccw90(pT->input.width/div_x,
                                                      pT->input.height/div_y,
                                                      pT->input.pData[p],
                                                      pT->output.pData[p]);
                        }
                    }
                    else
                        processed -= 1;
                    break;
                }
                case DVP_KN_YUV_UYVY_ROTATE_CW_90:
                case DVP_KN_YUV_UYVY_ROTATE_CCW_90:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == pT->output.color &&
                        pT->input.color == FOURCC_UYVY &&
                        pT->input.width == pT->output.height &&
                        pT->input.height == pT->output.width)
                    {
                        if (kernel == DVP_KN_YUV_UYVY_ROTATE_CW_90)
                            __uyvy_rotate_cw90(pT->input.width,
                                               pT->input.height,
                                               pT->input.pData[0],
                                               pT->output.pData[0],
                                               pT->input.y_stride,
                                               pT->output.y_stride);
                        else if (kernel == DVP_KN_YUV_UYVY_ROTATE_CCW_90)
                            __uyvy_rotate_ccw90(pT->input.width,
                                                pT->input.height,
                                                pT->input.pData[0],
                                                pT->output.pData[0],
                                                pT->input.y_stride,
                                                pT->output.y_stride);
                    }
                    else
                        processed -= 1;
                    break;
                }
                case DVP_KN_Y800_TO_XYXY:
                case DVP_KN_YUV_Y800_TO_XYXY:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == FOURCC_Y800 &&
                        pT->output.color == FOURCC_UYVY)
                    {
                        __luma_to_uyvy_image(pT->input.width,
                                             pT->input.height,
                                             pT->input.pData[0],
                                             pT->input.y_stride,
                                             pT->output.pData[0],
                                             pT->output.y_stride);
                    }
                    break;
                }
                case DVP_KN_UYVY_TO_BGR:
                case DVP_KN_YUV_UYVY_TO_BGR:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == FOURCC_UYVY &&
                        pT->output.color == FOURCC_BGR)
                    {
                        __uyvy_to_bgr_image_bt601(pT->input.width, pT->input.height,
                                                  pT->input.pData[0], pT->input.y_stride,
                                                  pT->output.pData[0], pT->output.y_stride);
                    }
                    break;
                }
                case DVP_KN_YUV_ARGB_TO_UYVY:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == FOURCC_ARGB &&
                        pT->output.color == FOURCC_UYVY)
                    {
                        DVP_PrintImage(DVP_ZONE_KGM, &pT->input);
                        DVP_PrintImage(DVP_ZONE_KGM, &pT->output);
                        __argb_to_uyvy_image_bt601(pT->input.width, pT->input.height,
                                                   pT->input.pData[0], pT->input.y_stride,
                                                   pT->output.pData[0], pT->output.y_stride);
                    }
                    break;
                }
#endif // YUV ONLY CASES

#if defined(DVP_USE_YUV) || defined(DVP_USE_VLIB)
                //******************************************************
                // YUV OR VLIB CASES
                //******************************************************


                case DVP_KN_UYVY_TO_YUV420p:
#if defined(DVP_USE_YUV)
                case DVP_KN_YUV_UYVY_TO_IYUV:
#elif defined(DVP_USE_VLIB)
                case DVP_KN_VLIB_UYVY_TO_YUV420p:
#endif
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == FOURCC_UYVY &&
                       (pT->output.color == FOURCC_IYUV || pT->output.color == FOURCC_YV12))
                    {
                        DVP_U08 *pU, *pV;
                        int y_stride = pT->output.y_stride;
                        int uv_stride = y_stride >> 1;
#if defined(DVP_USE_TILER)
                        if (y_stride == TILER_STRIDE_8BIT)
                            uv_stride = y_stride;
#endif
                        if (pT->output.color == FOURCC_YV12)
                        {
                            pV = pT->output.pData[1];
                            pU = pT->output.pData[2];
                        }
                        else
                        {
                            pU = pT->output.pData[1];
                            pV = pT->output.pData[2];
                        }
#if defined(DVP_USE_YUV)
                        __uyvy_to_yuv420_image(pT->input.width,
                                               pT->input.height,
                                               pT->input.pData[0],
                                               pT->input.y_stride,
                                               pT->output.pData[0],
                                               pU,
                                               pV,
                                               y_stride,
                                               uv_stride);
#elif defined(DVP_USE_VLIB)
                        if (uv_stride != pT->output.y_stride)
                        {
                            VLIB_convertUYVYint_to_YUV420pl(pT->input.pData[0],
                                                      pT->input.width,
                                                      pT->input.y_stride/pT->input.x_stride,
                                                      pT->input.height,
                                                      pT->output.pData[0],
                                                      pV,
                                                      pU);
                        }
                        else
                        {
                            DVP_U08* in  = pT->input.pData[0];
                            DVP_U08* Y   = pT->output.pData[0];
                            for(i=0; i<pT->input.height/2; i++)
                            {
                                VLIB_convertUYVYint_to_YUV420pl(in,
                                                          pT->input.width,
                                                          pT->input.y_stride/pT->input.x_stride,
                                                          2,
                                                          Y,
                                                          pV,
                                                          pU);
                                in  += pT->input.y_stride*2;
                                Y += pT->output.y_stride*2;
                                pU += uv_stride;
                                pV += uv_stride;
                            }
                        }
#endif
                    }
                    break;
                }
                case DVP_KN_UYVY_TO_RGBp:
#if defined(DVP_USE_YUV)
                case DVP_KN_YUV_UYVY_TO_RGBp:
#elif defined(DVP_USE_VLIB)
                case DVP_KN_VLIB_UYVY_TO_RGBp:
#endif
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == FOURCC_UYVY &&
                        pT->output.color == FOURCC_RGBP)
                    {
#if defined(DVP_USE_YUV)
                        __uyvy_to_rgbp_image_bt601(pT->input.width,
                                                   pT->input.height,
                                                   pT->input.pData[0],
                                                   pT->input.y_stride,
                                                   pT->output.pData[0],
                                                   pT->output.pData[1],
                                                   pT->output.pData[2],
                                                   pT->output.y_stride);
#elif defined(DVP_USE_VLIB)
                        // Convert UYVY to RGB (planar)
                        S16 coeff[] = {8192, 11229, -2757, -5720, 14192}; // 2^13*coeff
                        VLIB_convertUYVYint_to_RGBpl(pT->input.pData[0],
                                                     pT->input.width,
                                                     pT->input.y_stride/pT->input.x_stride,
                                                     pT->input.height,
                                                     coeff,
                                                     pT->output.pData[0],
                                                     pT->output.pData[1],
                                                     pT->output.pData[2]);
#endif
                    }
                    break;
                }

                case DVP_KN_XYXY_TO_Y800:
#if defined(DVP_USE_YUV)
                case DVP_KN_YUV_XYXY_TO_Y800:
#endif
#if defined(DVP_USE_VLIB)
                case DVP_KN_VLIB_XYXY_TO_Y800:
#endif
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == FOURCC_UYVY)
                    {
#if defined(DVP_USE_YUV)
                        __uyvy_luma_extract(pT->input.width, pT->input.height,
                                            pT->input.pData[0], pT->input.y_stride,
                                            pT->output.pData[0], pT->output.y_stride);
#elif defined(DVP_USE_VLIB)
                        VLIB_extractLumaFromxYxY(pT->input.pData[0],
                                                 pT->input.width,
                                                 pT->input.y_stride/pT->input.x_stride,
                                                 pT->input.height,
                                                 pT->output.pData[0]);
#endif
                    }
                    break;
                }

                case DVP_KN_UYVY_TO_YUV444p:
#if defined(DVP_USE_YUV)
                case DVP_KN_YUV_UYVY_TO_YUV444p:
#elif defined(DVP_USE_VLIB)
                case DVP_KN_VLIB_UYVY_TO_YUV444p:
#endif
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    // the conversion call is based on the input to output formats
                    if (pT->input.color == FOURCC_UYVY &&
                        (pT->output.color == FOURCC_YV24 || pT->output.color == FOURCC_YU24))
                    {
                        // Convert UYVY to YUV444(planar)
                        DVP_U08 *pU, *pV;
                        if (pT->output.color == FOURCC_YV24)
                        {
                            pV = pT->output.pData[1];
                            pU = pT->output.pData[2];
                        }
                        else
                        {
                            pU = pT->output.pData[1];
                            pV = pT->output.pData[2];
                        }
#if defined(DVP_USE_YUV)
                        __uyvy_to_yuv444p_image(pT->input.width, pT->input.height,
                                                pT->input.pData[0], pT->input.y_stride,
                                                pT->output.pData[0],
                                                pU,
                                                pV,
                                                pT->output.y_stride);
#elif defined(DVP_USE_VLIB)
                        VLIB_convertUYVYint_to_YUV444pl(pT->input.pData[0],
                                                        pT->input.width,
                                                        pT->input.y_stride/pT->input.x_stride,
                                                        pT->input.height,
                                                        pT->output.pData[0],
                                                        pV,
                                                        pU);
#endif
                    }
                    break;
                }
#endif // YUV OR VLIB

#if defined(DVP_USE_VLIB)
                //******************************************************
                // VLIB ONLY CASES
                //******************************************************

                case DVP_KN_YXYX_TO_Y800:
#if defined(DVP_USE_YUV)
                case DVP_KN_YUV_YXYX_TO_Y800:
#endif
#if defined(DVP_USE_VLIB)
                case DVP_KN_VLIB_YXYX_TO_Y800:
#endif
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == FOURCC_YUY2)
                    {
#if defined(DVP_USE_YUV)
                        __yuyv_luma_extract(pT->input.width, pT->input.height,
                                            pT->input.pData[0], pT->input.y_stride,
                                            pT->output.pData[0], pT->output.y_stride);
#elif defined(DVP_USE_VLIB)
                        VLIB_extractLumaFromYxYx(pT->input.pData[0],
                                                 pT->input.width,
                                                 pT->input.y_stride/pT->input.x_stride,
                                                 pT->input.height,
                                                 pT->output.pData[0]);
#endif
                    }
                    break;
                }
                case DVP_KN_YUV422p_TO_UYVY:
                case DVP_KN_VLIB_YUV422p_TO_UYVY:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if ((pT->input.color == FOURCC_YV16 || pT->input.color == FOURCC_YU16) &&
                             pT->output.color == FOURCC_UYVY)
                    {
                        DVP_U08 *pU, *pV;
                        if (pT->input.color == FOURCC_YV16)
                        {
                            pV = pT->input.pData[1];
                            pU = pT->input.pData[2];
                        }
                        else
                        {
                            pU = pT->input.pData[1];
                            pV = pT->input.pData[2];
                        }
                        VLIB_ConvertUYVYpl_to_YUVint(pT->input.pData[0],pU,pV,
                                                     pT->input.width,
                                                     pT->input.y_stride/pT->input.x_stride,
                                                     pT->input.height,
                                                     pT->output.pData[0]);
                    }
                    break;
                }
                case DVP_KN_UYVY_TO_YUV422p:
                case DVP_KN_VLIB_UYVY_TO_YUV422p:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (pT->input.color == FOURCC_UYVY &&
                       (pT->output.color == FOURCC_YV16 || pT->output.color == FOURCC_YU16))
                    {
                        DVP_U08 *pU, *pV;
                        if (pT->output.color == FOURCC_YV16)
                        {
                            pV = pT->output.pData[1];
                            pU = pT->output.pData[2];
                        }
                        else
                        {
                            pU = pT->output.pData[1];
                            pV = pT->output.pData[2];
                        }
                        VLIB_convertUYVYint_to_YUV422pl(pT->input.pData[0],
                                                        pT->input.width,
                                                        pT->input.y_stride/pT->input.x_stride,
                                                        pT->input.height,
                                                        pT->output.pData[0],pU,pV);

                    }
                    break;
                }
#endif

#if defined(DVP_USE_YUV) || defined(DVP_USE_IMAGE)
                // YUV OR IMAGE_t

                case DVP_KN_BGR3_TO_UYVY:
                case DVP_KN_BGR3_TO_IYUV:
#if defined(DVP_USE_YUV)
                case DVP_KN_YUV_BGR_TO_UYVY:
                case DVP_KN_YUV_BGR_TO_IYUV:
#endif
#if !defined(DVP_USE_YUV) && defined(DVP_USE_IMAGE)
                case DVP_KN_NV12_TO_YUV444p:
#endif
#if defined(DVP_USE_IMAGE)
                case DVP_KN_YUV444p_TO_UYVY:
                case DVP_KN_NV12_TO_UYVY:
                case DVP_KN_BGR3_TO_NV12:
#endif
                {
#if defined(DVP_USE_YUV)
                    DVP_Transform_t *pIO = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    if (kernel == DVP_KN_YUV_BGR_TO_UYVY ||
                        kernel == DVP_KN_BGR3_TO_UYVY)
                    {
                        __bgr_to_uyvy_image_bt601(pIO->input.width,
                                                  pIO->input.height,
                                                  pIO->input.pData[0],
                                                  pIO->input.y_stride,
                                                  pIO->output.pData[0],
                                                  pIO->output.y_stride);
                        break;
                    }
                    else if (kernel == DVP_KN_YUV_BGR_TO_IYUV ||
                             kernel == DVP_KN_BGR3_TO_IYUV)
                    {
                         __bgr_to_iyuv_image_bt601(pIO->input.width,
                                                   pIO->input.height,
                                                   pIO->input.pData[0],
                                                   pIO->input.y_stride,
                                                   pIO->output.pData[0],
                                                   pIO->output.pData[1],
                                                   pIO->output.pData[2]);
                        break;
                    }
                    else
#endif
                    {
                        image_t src, dst;
                        DVP_Image_to_image_t(&src, &dvp_knode_to(&pSubNodes[n], DVP_Transform_t)->input);
                        DVP_Image_to_image_t(&dst, &dvp_knode_to(&pSubNodes[n], DVP_Transform_t)->output);
                        image_convert(&dst, &src);
                        break;
                    }
                }


#endif

#if defined(DVP_USE_VLIB)
#if defined(VLIB_DISPARITY_FIXED)
                // these disparity calculations seem to be broken, we'll reenable once they are fixed.
                case DVP_KN_VLIB_DISPARITY_SAD8:
                {
                    DVP_U32 y = 0, d = 0;
                    DVP_Disparity_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Disparity_t);
                    S32 disparityWindow = (20*pT->left.width)/100; // 20% of the width
                    U08 *scratch = (U08 *)calloc(1, pT->left.width);
                    if (scratch)
                    {
                        for (d = 1; d < pT->displacement; d++)
                        {
                            memset(scratch, 0, pT->left.width);
                            VLIB_disparity_SAD_firstRow8(&pT->left.pData[0][disparityWindow/2],
                                                         &pT->right.pData[0][disparityWindow/2],
                                                         pT->cost.pData[0],
                                                         pT->mincost.pData[0],
                                                         scratch,
                                                         pT->disparity.pData[0],
                                                         d,
                                                         pT->left.width,
                                                         pT->left.y_stride,
                                                         disparityWindow);

                            for (y = 1; y < pT->left.height; y++)
                            {
                                VLIB_disparity_SAD8(&pT->left.pData[0][y*pT->left.y_stride + disparityWindow/2],
                                                    &pT->right.pData[0][y*pT->right.y_stride + disparityWindow/2],
                                                    (U16 *)&pT->cost.pData[0][y*pT->cost.y_stride],
                                                    (U16 *)&pT->mincost.pData[0][y*pT->mincost.y_stride],
                                                    (S08 *)&pT->disparity.pData[0][y*pT->disparity.y_stride],
                                                    d,
                                                    pT->left.width,
                                                    pT->left.y_stride,
                                                    disparityWindow);
                            }

                        }
                        free(scratch);
                    }
                    break;
                }
                case DVP_KN_VLIB_DISPARITY_SAD16:
                {
                    DVP_U32 y = 0;
                    DVP_Disparity_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Disparity_t);
                    for (y = 0; y < pT->left.height; y++)
                    {
                        VLIB_disparity_SAD16((U16 *)&pT->left.pData[0][y*pT->left.y_stride],
                                             (U16 *)&pT->right.pData[0][y*pT->right.y_stride],
                                             (U16 *)&pT->cost.pData[0][y*pT->cost.y_stride],
                                             (U16 *)&pT->mincost.pData[0][y*pT->mincost.y_stride],
                                             (S08 *)&pT->disparity.pData[0][y*pT->disparity.y_stride],
                                             pT->displacement,
                                             pT->left.width,
                                             pT->left.y_stride/pT->left.x_stride,
                                             1);
                    }
                    break;
                }
#endif
                case DVP_KN_VLIB_UYVY_TO_HSLp:       //Supports both 2D and 1D inputs/outputs (160x120!!)
                {
                    DVP_Int2Pl_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Int2Pl_t);

                    if (pT->input.color == FOURCC_UYVY)
                    {
                        DVP_U08* in  = pT->input.pData[0];
                        DVP_U08* out0 = pT->output1.pData[0];
                        DVP_U08* out1 = pT->output2.pData[0];
                        DVP_U08* out2 = pT->output3.pData[0];
                        DVP_U32 inStride = pT->input.y_stride;
                        DVP_U32 outStride1 = pT->output1.y_stride;
                        DVP_U32 outStride2 = pT->output2.y_stride;
                        if(outStride1 > pT->output1.width)     // If the output does not have stride == width, call the function in a loop
                        {
                            for(i=0; i<pT->input.height; i++)
                            {
                                VLIB_convertUYVYint_to_HSLpl( in,
                                                          pT->input.width,
                                                          1,
                                                          1,
                                                          yuv2rgbMatrix,
                                                          div_table_DDR2,
                                                          (DVP_U16*)out0,
                                                          out1,
                                                          out2);
                                in  += inStride;
                                out0 += outStride1;
                                out1 += outStride2;
                                out2 += outStride2;
                            }
                        }
                        else // The VLIB function can handle strided inputs, but not outputs
                        {
                            VLIB_convertUYVYint_to_HSLpl( in,
                                                      pT->input.width,
                                                      inStride/pT->input.x_stride,
                                                      pT->input.height,
                                                      yuv2rgbMatrix,
                                                      div_table_DDR2,
                                                      (DVP_U16*)out0,
                                                      out1,
                                                      out2);
                        }
                    }
                    break;
                }
                case DVP_KN_VLIB_UYVY_TO_LABp:
                {
                    DVP_Int2Pl_t *pLAB = dvp_knode_to(&pSubNodes[n], DVP_Int2Pl_t);

                    if (pLAB->input.color == FOURCC_UYVY)
                    {
                        DVP_U08* in  = pLAB->input.pData[0];
                        DVP_U08* L = pLAB->output1.pData[0];
                        DVP_U08* a = pLAB->output2.pData[0];
                        DVP_U08* b = pLAB->output3.pData[0];
                        DVP_U08* LUT = pLAB->scratch.pData;
                        DVP_U32 inStride = pLAB->input.y_stride;
                        DVP_U32 outStride = pLAB->output1.y_stride;

                        if(outStride > pLAB->output1.width)     // If the output does not have stride == width, call the function in a loop
                        {
                            VLIB_initUYVYint_to_LABpl_LUT((S32) pLAB->factor,
                                                yuv2rgbMatrix,
                                                whitePoint,
                                                (U16*)LUT);

                            for(i=0; i<pLAB->input.height; i++)
                            {
                                VLIB_convertUYVYint_to_LABpl_LUT( in,
                                                          pLAB->input.width,
                                                          pLAB->input.width,
                                                          1,
                                                          (S32) pLAB->factor,
                                                          (U16*)LUT,
                                                          (U16*)L,
                                                          (U16*)a,
                                                          (U16*)b);
                                in  += inStride;
                                L += outStride;
                                a += outStride;
                                b += outStride;
                            }
                        }
                        else // The VLIB function can handle strided inputs, but not outputs
                        {
                            VLIB_convertUYVYint_to_LABpl_LUT( in,
                                                      pLAB->input.width,
                                                      pLAB->input.width,
                                                      pLAB->input.height,
                                                      (S32) pLAB->factor,
                                                      (U16*)LUT,
                                                      (U16*)L,
                                                      (U16*)a,
                                                      (U16*)b);
                        }
                    }
                    break;
                }
                case DVP_KN_VLIB_INIT_MEAN_LUMA_S16:
                {
                    DVP_U32 y = 0;
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    for (y = 0; y < pT->input.height; y++)
                    {
                        VLIB_initMeanWithLumaS16((S16 *)&pT->output.pData[0][y*pT->output.y_stride],
                                                 &pT->input.pData[0][y*pT->input.y_stride],
                                                 pT->input.width);
                    }
                    break;
                }
                case DVP_KN_VLIB_INIT_MEAN_LUMA_S32:
                {
                    DVP_U32 y = 0;
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    for (y = 0; y < pT->input.height; y++)
                    {
                        VLIB_initMeanWithLumaS32((S32 *)&pT->output.pData[0][y*pT->output.y_stride],
                                                 &pT->input.pData[0][y*pT->input.y_stride],
                                                 pT->input.width);
                    }
                    break;
                }
                case DVP_KN_VLIB_INIT_VAR_CONST_S16:
                {
                    DVP_U32 y = 0;
                    DVP_ConstVar_t *pV = dvp_knode_to(&pSubNodes[n], DVP_ConstVar_t);
                    for (y = 0; y < pV->var.height; y++)
                    {
                        VLIB_initVarWithConstS16((S16 *)&pV->var.pData[0][y*pV->var.y_stride],
                                                 pV->constVar,
                                                 pV->var.width);
                    }
                    break;
                }
                case DVP_KN_VLIB_INIT_VAR_CONST_S32:
                {
                    DVP_U32 y = 0;
                    DVP_ConstVar_t *pV = dvp_knode_to(&pSubNodes[n], DVP_ConstVar_t);
                    for (y = 0; y < pV->var.height; y++)
                    {
                        VLIB_initVarWithConstS32((S32 *)&pV->var.pData[0][y*pV->var.y_stride],
                                                 pV->constVar,
                                                 pV->var.width);
                    }
                    break;
                }
#endif

#if defined(DVP_USE_IMGFILTER) || defined(DVP_USE_VLIB)
                //*************************************
                // IMGFILTER OR VLIB
                //*************************************

                case DVP_KN_SOBEL_8:
#if defined(DVP_USE_IMGFILTER)
                case DVP_KN_IMGFILTER_SOBEL:
#elif defined(DVP_USE_VLIB)
                case DVP_KN_VLIB_SOBEL_8:
#endif
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
#if defined(DVP_USE_IMGFILTER)
                    DVP_imgFilter(&pT->input, DVP_IMGFILTER_SOBEL, &pT->output);
#elif defined(DVP_USE_VLIB)
                    VLIB_sobelFilter_U8(pT->input.pData[0],
                                        pT->input.width,
                                        pT->input.height,
                                        pT->input.y_stride,
                                        pT->output.pData[0]);
#endif
                    break;
                }
                case DVP_KN_SCHARR_8:
#if defined(DVP_USE_IMGFILTER)
                case DVP_KN_IMGFILTER_SCHARR:
#elif defined(DVP_USE_VLIB)
                case DVP_KN_VLIB_SCHARR_8:
#endif
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
#if defined(DVP_USE_IMGFILTER)
                    DVP_imgFilter(&pT->input, DVP_IMGFILTER_SCHARR, &pT->output);
#elif defined(DVP_USE_VLIB)
                    VLIB_scharrFilter_U8(pT->input.pData[0],
                                         pT->input.width,
                                         pT->input.height,
                                         pT->input.y_stride,
                                         pT->output.pData[0]);
#endif
                    break;
                }
                case DVP_KN_KROON_8:
#if defined(DVP_USE_IMGFILTER)
                case DVP_KN_IMGFILTER_KROON:
#elif defined(DVP_USE_VLIB)
                case DVP_KN_VLIB_KROON_8:
#endif
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
#if defined(DVP_USE_IMGFILTER)
                    DVP_imgFilter(&pT->input, DVP_IMGFILTER_KROON, &pT->output);
#elif defined(DVP_USE_VLIB)
                    VLIB_kroonFilter_U8(pT->input.pData[0],
                                        pT->input.width,
                                        pT->input.height,
                                        pT->input.y_stride,
                                        pT->output.pData[0]);
#endif
                    break;
                }
                case DVP_KN_PREWITT_8:
#if defined(DVP_USE_IMGFILTER)
                case DVP_KN_IMGFILTER_PREWITT:
#elif defined(DVP_USE_VLIB)
                case DVP_KN_VLIB_PREWITT_8:
#endif
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
#if defined(DVP_USE_IMGFILTER)
                    DVP_imgFilter(&pT->input, DVP_IMGFILTER_PREWITT, &pT->output);
#elif defined(DVP_USE_VLIB)
                    VLIB_prewittFilter_U8(pT->input.pData[0],
                                          pT->input.width,
                                          pT->input.height,
                                          pT->input.y_stride,
                                          pT->output.pData[0]);
#endif
                    break;
                }
#endif // IMGFILTER || VLIB

#if defined(DVP_USE_RVM)
                case DVP_KN_RVM:
                {
                    DVP_Rvm_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Rvm_t);
                    Rvm_Image_t input, output;
                    RVM_colorformat_e color;
                    DVP_U32 scratchSize;
                    void *scratch;

                    input.pData[0] = pT->input.pData[0];
                    input.stride[0] = pT->input.y_stride;
                    output.pData[0] = pT->output.pData[0];
                    output.stride[0] = pT->output.y_stride;

                    if(pT->input.color == FOURCC_UYVY)
                        color = RVM_COLORFORMAT_YUV422;
                    else if (pT->input.color == FOURCC_Y800)
                        color = RVM_COLORFORMAT_LUMA8;
                    else if (pT->input.color == FOURCC_NV12)
                    {
                        color = RVM_COLORFORMAT_NV12;
                        input.pData[1] = pT->input.pData[1];
                        input.stride[1] = pT->input.y_stride;
                        output.pData[1] = pT->output.pData[1];
                        output.stride[1] = pT->output.y_stride;
                    }
                    else
                    {
                        processed--;
                        pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                        break;
                    }

                    scratchSize = RVM_getScratchSize(pT->lut.pData, color);
                    scratch = malloc(scratchSize);

                    if(scratch)
                    {
                        RVM_wrapper(&input,
                                    &output,
                                    color,
                                    pT->lut.pData,
                                    (uint8_t *)scratch,
                                    scratchSize);
                        free(scratch);
                    }
                    else
                    {
                        processed--;
                        pSubNodes[n].header.error = DVP_ERROR_NO_MEMORY;
                    }
                    break;
                }
#endif
#if defined(DVP_USE_IMGLIB)
                case DVP_KN_SOBEL_3x3_8:
                case DVP_KN_IMG_SOBEL_3x3_8:
                {
                    DVP_Transform_t *pIO = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);

                    if (pIO->input.y_stride == (int)pIO->input.width)
                    {
                        IMG_sobel_3x3_8(pIO->input.pData[0],
                                        pIO->output.pData[0],
                                        pIO->input.width,
                                        pIO->input.height);
                    }
                    else
                    {
                        DVP_U08 *tmpBuf = (DVP_U08 *)malloc(pIO->input.width * (16 + (3 -1)));
                        DVP_U08* out = pIO->output.pData[0];
                        DVP_U32 y;
                        DVP_U32  newheight = pIO->input.height -(3 -1);
                        DVP_U08 blockHeight = 16;

                        for(y=0; y<newheight; y+=blockHeight)
                        {
                            if(y+blockHeight > newheight)
                                blockHeight = newheight-y;

                            for(i=0; i<((DVP_U32)blockHeight + (3 -1)); i++)
                                memcpy(&tmpBuf[i*pIO->input.width], &pIO->input.pData[0][(y+i)*pIO->input.y_stride], pIO->input.width);

                            IMG_sobel_3x3_8(tmpBuf,
                                    &out[y*pIO->output.y_stride],
                                    pIO->input.width,
                                    blockHeight+2);
                        }
                        free(tmpBuf);
                    }

                    break;
                }
                case DVP_KN_SOBEL_3x3_16s:
                case DVP_KN_IMG_SOBEL_3x3_16s:
                {
                    DVP_Transform_t *pSb = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);

                    IMG_sobel_3x3_16s((short *)pSb->input.pData[0],
                                 (short *)pSb->output.pData[0],
                                 (short)(pSb->input.width),
                                 (short)(pSb->input.height ) );
                    break;
                }
                case DVP_KN_SOBEL_5x5_16s:
                case DVP_KN_IMG_SOBEL_5x5_16s:
                {
                    DVP_Transform_t *pSb = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);

                    IMG_sobel_5x5_16s((short *)pSb->input.pData[0],
                                 (short *)pSb->output.pData[0],
                                 (short)(pSb->input.width) ,
                                 (short)(pSb->input.height ));
                    break;
                }
                case DVP_KN_SOBEL_7x7_16s:
                case DVP_KN_IMG_SOBEL_7x7_16s:
                {
                    DVP_Transform_t *pSb = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);

                    IMG_sobel_7x7_16s((short *)pSb->input.pData[0],
                                 (short *)pSb->output.pData[0],
                                 (short)(pSb->input.width) ,
                                 (short)(pSb->input.height ) );
                    break;
                }
                case DVP_KN_CONV_3x3: // Assumes 1D buffer
                case DVP_KN_IMG_CONV_3x3: // Assumes 1D buffer
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_t);
                    DVP_U32  filterSize = 3;
                    DVP_U32  newheight = pImg->input.height - (filterSize -1);
                    DVP_U32  newwidth =  pImg->input.width - (filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];

                    for(i=0; i<newheight; i++)
                    {
                        IMG_conv_3x3_i8_c8s(in,
                                            out,
                                            (short)newwidth,
                                            (short)pImg->input.y_stride,
                                            (const char*)pImg->mask.pData[0],
                                            (int)pImg->shiftMask);
                        in += pImg->input.y_stride;
                        out += pImg->output.y_stride;
                    }

                    break;
                }
                case DVP_KN_CONV_5x5:  // 1D or 2D buffers
                case DVP_KN_IMG_CONV_5x5:  // 1D or 2D buffers
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_t);
                    DVP_U32  filterSize = 5;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];

                    for(i=0; i<newheight; i++)
                    {
                        IMG_conv_5x5_i8_c8s(in,
                                                    out,
                                                    (short)(pImg->input.width -(filterSize -1)),
                                                    (short)pImg->input.y_stride,
                                                    (const char *)pImg->mask.pData[0],
                                                    (short)pImg->shiftMask);
                        in += pImg->input.y_stride;
                        out += pImg->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_CONV_7x7:  // 1D or 2D buffers
                case DVP_KN_IMG_CONV_7x7:  // 1D or 2D buffers
                case DVP_KN_CANNY_IMAGE_SMOOTHING:
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_t);
                    DVP_U32  filterSize = 7;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];

                    for(i=0; i<newheight; i++)
                    {
                        IMG_conv_7x7_i8_c8s(in,
                                            out,
                                            (short)(pImg->input.width -(filterSize -1)),
                                            (short)pImg->input.y_stride,
                                            (const signed char *)pImg->mask.pData[0],
                                            (short)pImg->shiftMask);
                        in += pImg->input.y_stride;
                        out += pImg->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_IMG_CONV_3x3_I16s_C16:
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_t);
                    DVP_U32  filterSize = 3;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;

                    for(i=0; i<newheight; i++)
                    {
                        IMG_conv_3x3_i16s_c16s((short*)in,
                                                (short*)out,
                                                (short)(pImg->input.width -(filterSize -1)),
                                                (int)pImg->input.width,
                                                (short*)pImg->mask.pData[0],
                                                (int)pImg->shiftMask);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CONV_5x5_I8_C16:
                {
                    DVP_ImageConvolution_with_buffer_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_with_buffer_t);
                    DVP_U32  filterSize = 5;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_conv_5x5_i8_c16s(in,
                                            out,
                                            (short)(pImg->input.width -(filterSize -1)),
                                            (short)pImg->input.width,
                                            (DVP_S16*)pImg->mask.pData[0],
                                            (short)pImg->shiftMask);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CONV_5x5_I16s_C16:
                {
                    DVP_ImageConvolution_with_buffer_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_with_buffer_t);
                    DVP_U32  filterSize = 5;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_conv_5x5_i16s_c16s((short*)in,
                                                (short*)out,
                                                (short)(pImg->input.width -(filterSize -1)),
                                                (short)pImg->input.width,
                                                (short*)pImg->mask.pData[0],
                                                (short)pImg->shiftMask);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CONV_7x7_I8_C16:
                {
                    DVP_ImageConvolution_with_buffer_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_with_buffer_t);
                    DVP_U32  filterSize = 7;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_conv_7x7_i8_c16s(in,
                                            out,
                                            (short)(pImg->input.width -(filterSize -1)),
                                            (short)pImg->input.width,
                                            (short*)pImg->mask.pData[0],
                                            (short)pImg->shiftMask);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CONV_7x7_I16s_C16:
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_t);
                    DVP_U32  filterSize = 7;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_conv_7x7_i16s_c16s((short*)in,
                                                (short*)out,
                                                (short)(pImg->input.width -(filterSize -1)),
                                                (short)pImg->input.width,
                                                (short*)pImg->mask.pData[0],
                                                (short)pImg->shiftMask);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CONV_11x11:
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_t);
                    DVP_U32  filterSize = 11;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_conv_11x11_i8_c8s((DVP_U08*)in,
                                                (DVP_U08*)out,
                                                (short)(pImg->input.width -(filterSize -1)),
                                                (short)pImg->input.width,
                                                (char*)pImg->mask.pData[0],
                                                (short)pImg->shiftMask);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CONV_11x11_I16s_C16:
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_t);
                    DVP_U32  filterSize = 11;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_conv_11x11_i16s_c16s((short*)in,
                                                (short*)out,
                                                (short)(pImg->input.width -(filterSize -1)),
                                                (short)pImg->input.width,
                                                (short*)pImg->mask.pData[0],
                                                (short)pImg->shiftMask);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CORR_3x3:
                {
                    DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                    DVP_U32  filterSize = 3;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_corr_3x3_i8_c8(in,
                                        (int*)out,
                                        (int)pImg->input.width - (filterSize - 1),
                                        (int)pImg->input.width,
                                        (DVP_U08*)pImg->mask.pData[0],
                                        (int)pImg->shift,
                                        (int)pImg->round);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CORR_3x3_I8_C16s:
                {
                    DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                    DVP_U32  filterSize = 3;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_corr_3x3_i8_c16s(in,
                                        (int*)out,
                                        (short)pImg->input.width - (filterSize - 1),
                                        (short)pImg->input.width,
                                        (short*)pImg->mask.pData[0]);

                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CORR_3x3_I16s_C16s:
                {
                    DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                    DVP_U32  filterSize = 3;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_corr_3x3_i16s_c16s((short*)in,
                                        (int*)out,
                                        (short)pImg->input.width - (filterSize - 1),
                                        (short)pImg->input.width,
                                        (short*)pImg->mask.pData[0],
                                        (short)pImg->shift,
                                        (int)pImg->round);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CORR_5x5_I16s_C16s:
                {
                    DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                    DVP_U32  filterSize = 5;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_corr_5x5_i16s_c16s((short*)in,
                                        (int*)out,
                                        (short)pImg->input.width - (filterSize - 1),
                                        (short)pImg->input.width,
                                        (short*)pImg->mask.pData[0],
                                        (short)pImg->shift,
                                        (int)pImg->round);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CORR_11x11_I8_C16s: //yet to be defined
                {
                    DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                    DVP_U32  filterSize = 11;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_corr_11x11_i8_c16s(in,
                                        (int*)out,
                                        (short)pImg->input.width - (filterSize - 1),
                                        (short)pImg->input.width,
                                        (short*)pImg->mask.pData[0]);

                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CORR_11x11_I16s_C16s:
                {
                    DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                    DVP_U32  filterSize = 11;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_corr_11x11_i16s_c16s((short*)in,
                                        (int*)out,
                                        (short)pImg->input.width - (filterSize - 1),
                                        (short)pImg->input.width,
                                        (short*)pImg->mask.pData[0],
                                        (short)pImg->shift,
                                        (int)pImg->round);
                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CORR_GEN_I16s_C16s:
                {
                    DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                    DVP_U32  filterSize = pImg->m;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_corr_gen_i16s_c16s((short*)in,
                                            (short*)pImg->mask.pData[0],
                                            (short*)out,
                                            filterSize,
                                            (short)pImg->input.width);


                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_CORR_GEN_IQ:
                {
                    DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                    DVP_U32  filterSize = pImg->m;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    DVP_U32 outStride = pImg->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_corr_gen_iq((int*)in,
                                        (short*)pImg->mask.pData[0],
                                        (int*)out,
                                        filterSize,
                                        (short)pImg->input.width,
                                        pImg->x_qpt,
                                        pImg->h_qpt,
                                        pImg->y_qpt);


                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_HISTOGRAM_8:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                    IMG_histogram_8(pH->input.pData[0],
                                    pH->input.width * pH->input.height,
                                     (int)pH->binWeight,
                                     (short*)pH->histArray.pData,
                                     (short*)pH->hOut.pData);
                    break;
                }
                case DVP_KN_IMG_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                    memset(pH->histArray.pData, 0, (1 << pH->numBins)*4);
                    IMG_histogram_16((DVP_U16 *)pH->input.pData[0],
                                     pH->input.width * pH->input.height,
                                     (int)pH->binWeight,
                                     (short*)pH->histArray.pData,
                                     (short*)pH->hOut.pData,
                                     (int)pH->numBins);
                    break;
                }
                case DVP_KN_IMG_CLIPPING_16:
                {
                    DVP_CannyHystThresholding_t *pClp = dvp_knode_to(&pSubNodes[n], DVP_CannyHystThresholding_t);
                    IMG_clipping_16s((short *)pClp->inMag.pData[0],
                                 (short)(pClp->inMag.height),
                                 (short)(pClp->inMag.width),
                                 (short *)pClp->output.pData[0],
                                 (short)(pClp->hiThresh) ,
                                 (short)(pClp->loThresh ));
                    break;
                }
                case DVP_KN_IMG_MEDIAN_3x3_8:
                {
                    DVP_Transform_t *pMd =  dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    DVP_U08* in  = pMd->input.pData[0];
                    DVP_U08* out = pMd->output.pData[0];
                    DVP_U32 newheight = pMd->input.height - 2;
                    DVP_U32 inStride = pMd->input.y_stride;
                    DVP_U32 outStride = pMd->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_median_3x3_8(in,
                                      (int)pMd->input.width,
                                      (DVP_U08*)out);

                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_MEDIAN_3x3_16s:
                {
                    DVP_Transform_t *pMd =  dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    DVP_U08* in  = pMd->input.pData[0];
                    DVP_U08* out = pMd->output.pData[0];
                    DVP_U32 newheight = pMd->input.height - 2;
                    DVP_U32 inStride = pMd->input.y_stride;
                    DVP_U32 outStride = pMd->output.y_stride;
                    for(i=0; i<newheight; i++)
                    {
                        IMG_median_3x3_16s((DVP_S16*)in,
                                      (int)pMd->input.width,
                                      (DVP_S16*)out);

                        in += inStride;
                        out += outStride;
                    }
                    break;
                }
                case DVP_KN_IMG_PIX_EXPAND:
                {
                    DVP_Transform_t *pMd = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    IMG_pix_expand((int)(pMd->input.width*pMd->input.height),
                                 (DVP_U08 *)pMd->input.pData[0],
                                 (DVP_S16 *)pMd->output.pData[0] );
                    break;
                }
                case DVP_KN_IMG_PIX_SAT:
                {
                    DVP_Transform_t *pMd = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    IMG_pix_sat((int)(pMd->input.width*pMd->input.height),
                                 (DVP_S16 *)pMd->input.pData[0],
                                 (DVP_U08 *)pMd->output.pData[0] );
                    break;
                }
                case DVP_KN_IMG_YC_DEMUX_BE16_8:       //Supports both 2D and 1D inputs/outputs
                {
                    DVP_Int2Pl_t *pYc = dvp_knode_to(&pSubNodes[n], DVP_Int2Pl_t);
                    DVP_U08* in  = pYc->input.pData[0];
                    DVP_U08* y = (DVP_U08*)pYc->output1.pData[0];
                    DVP_U08* cr = (DVP_U08*)pYc->output2.pData[0];
                    DVP_U08* cb = (DVP_U08*)pYc->output3.pData[0];
                    DVP_U32 i;
                    for(i=0; i<pYc->input.height; i++)
                    {
                        IMG_yc_demux_be16_8(pYc->input.width,
                                           &in[i*pYc->input.y_stride],
                                           (DVP_S16*)&y[i*pYc->output1.y_stride],
                                           (DVP_S16*)&cr[i*pYc->output2.y_stride],
                                           (DVP_S16*)&cb[i*pYc->output3.y_stride]);
                    }
                    break;
                }
                case DVP_KN_IMG_YC_DEMUX_LE16_8:       //Supports both 2D and 1D inputs/outputs
                {
                    DVP_Int2Pl_t *pYc = dvp_knode_to(&pSubNodes[n], DVP_Int2Pl_t);
                    DVP_U08* in  = pYc->input.pData[0];
                    DVP_U08* y = (DVP_U08*)pYc->output1.pData[0];
                    DVP_U08* cr = (DVP_U08*)pYc->output2.pData[0];
                    DVP_U08* cb = (DVP_U08*)pYc->output3.pData[0];
                    DVP_U32 i;
                    for(i=0; i<pYc->input.height; i++)
                    {
                        IMG_yc_demux_le16_8(pYc->input.width,
                                           &in[i*pYc->input.y_stride],
                                           (DVP_S16*)&y[i*pYc->output1.y_stride],
                                           (DVP_S16*)&cr[i*pYc->output2.y_stride],
                                           (DVP_S16*)&cb[i*pYc->output3.y_stride]);
                   }
                    break;
                }
                case DVP_KN_IMG_YUV422p_TO_RGB565:       //Supports both 2D and 1D inputs/outputs
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    DVP_U08* y = pT->input.pData[0];
                    DVP_U08 *cb, *cr;
                    DVP_U08* rgb  = (DVP_U08*)pT->output.pData[0];
                    if (pT->input.color == FOURCC_YU16)
                    {
                        cb = pT->input.pData[1];
                        cr = pT->input.pData[2];
                    }
                    else
                    {
                        cr = pT->input.pData[1];
                        cb = pT->input.pData[2];
                    }

                    IMG_ycbcr422pl_to_rgb565( yuv2rgbMatrix,
                                          y,
                                          cb,
                                          cr,
                                          (DVP_U16*)rgb,
                                          pT->input.width*pT->input.height);
                   processed++;
                   break;
                }
                case DVP_KN_IMG_SAD_8x8: // Assumes 1D buffer
                {
                    DVP_SAD_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_SAD_t);
                    DVP_U32  filterSize = 8;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    IMG_sad_8x8_i8_c8(in,
                                    (unsigned char*)pImg->refImg.pData[0],
                                    out,
                                    (int)pImg->input.width,
                                    (int)newheight,
                                    (int)pImg->shiftMask,
                                    0,
                                    8);
                    break;
                }
                case DVP_KN_IMG_SAD_16x16: // Assumes 1D buffer
                {
                    DVP_SAD_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_SAD_t);
                    DVP_U32  filterSize = 16;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    IMG_sad_16x16_i8_c8(in,
                                    (unsigned char*)pImg->refImg.pData[0],
                                    out,
                                    (int)pImg->input.width,
                                    (int)newheight,
                                    (int)pImg->shiftMask,
                                    0,
                                    8);
                    break;
                }
                case DVP_KN_IMG_SAD_3x3: // Assumes 1D buffer
                {
                    DVP_SAD_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_SAD_t);
                    DVP_U32  filterSize = 3;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    IMG_sad_3x3_i8_c8(in,
                                    (unsigned char*)pImg->refImg.pData[0],
                                    out,
                                    (int)pImg->input.width,
                                    (int)newheight,
                                    0,
                                    3);
                    break;
                }
                case DVP_KN_IMG_SAD_5x5: // Assumes 1D buffer
                {
                    // DVP_SAD_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_SAD_t);
                    // DVP_U32  filterSize = 5;
                    // DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    // DVP_U08* in  = pImg->input.pData[0];
                    // DVP_U08* out = pImg->output.pData[0];
                    // IMG_sad_5x5_i8_c8(in,
                                    // (unsigned char*)pImg->refImg.pData[0],
                                    // out,
                                    // (int)pImg->input.width,
                                    // (int)newheight,
                                    // 0,
                                    // 5);
                    break;
                }
                case DVP_KN_IMG_SAD_7x7: // Assumes 1D buffer
                {
                    DVP_SAD_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_SAD_t);
                    DVP_U32  filterSize = 7;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    IMG_sad_7x7_i8_c8(in,
                                    (unsigned char*)pImg->refImg.pData[0],
                                    out,
                                    (int)pImg->input.width,
                                    (int)newheight,
                                    0,
                                    7);
                    break;
                }
                case DVP_KN_IMG_MAD_8x8: // Assumes 1D buffer
                {
                    DVP_SAD_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_SAD_t);
                    DVP_U32  filterSize = 8;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    IMG_mad_8x8((unsigned char*)in,
                                (unsigned char*)pImg->refImg.pData[0],
                                (int)pImg->input.width,
                                (int)pImg->input.width - (filterSize - 1),
                                (int)newheight,
                                (unsigned int*)out);
                    break;
                }
                case DVP_KN_IMG_MAD_16x16: // Assumes 1D buffer
                {
                    DVP_SAD_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_SAD_t);
                    DVP_U32  filterSize = 16;
                    DVP_U32  newheight = pImg->input.height -(filterSize -1);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U08* out = pImg->output.pData[0];
                    IMG_mad_16x16((unsigned char*)in,
                                    (unsigned char*)pImg->refImg.pData[0],
                                    (int)pImg->input.width,
                                    (int)pImg->input.width - (filterSize - 1),
                                    (int)newheight,
                                    (unsigned int*)out);
                    break;
                }
                case DVP_KN_THR_GT2MAX_8:
                case DVP_KN_IMG_THR_GT2MAX_8:
                {
                    DVP_Threshold_t *pThresh = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    DVP_U08* in  = pThresh->input.pData[0];
                    DVP_U08* out = pThresh->output.pData[0];

                    for(i=0; i<pThresh->input.height; i++)
                    {
                        IMG_thr_gt2max_8(in,
                                            out,
                                            (short)(pThresh->input.width),
                                            1,
                                            (unsigned char)(pThresh->thresh));
                        in += pThresh->input.y_stride;
                        out += pThresh->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_THR_GT2MAX_16:
                case DVP_KN_IMG_THR_GT2MAX_16:
                {
                    DVP_Threshold_t *pThresh = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    DVP_U08* in  = pThresh->input.pData[0];
                    DVP_U08* out = pThresh->output.pData[0];

                    for(i=0; i<pThresh->input.height; i++)
                    {
                        IMG_thr_gt2max_16((const unsigned short*)in,
                                            (unsigned short*)out,
                                            (short)(pThresh->input.width),
                                            1,
                                            (unsigned short)(pThresh->thresh));
                        in += pThresh->input.y_stride;
                        out += pThresh->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_THR_GT2THR_8:
                case DVP_KN_IMG_THR_GT2THR_8:
                {
                    DVP_Threshold_t *pThresh = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    DVP_U08* in  = pThresh->input.pData[0];
                    DVP_U08* out = pThresh->output.pData[0];

                    for(i=0; i<pThresh->input.height; i++)
                    {
                        IMG_thr_gt2thr_8(in,
                                            out,
                                            (short)(pThresh->input.width),
                                            1,
                                            (unsigned char)(pThresh->thresh));
                        in += pThresh->input.y_stride;
                        out += pThresh->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_THR_GT2THR_16:
                case DVP_KN_IMG_THR_GT2THR_16:
                {
                    DVP_Threshold_t *pThresh = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    DVP_U08* in  = pThresh->input.pData[0];
                    DVP_U08* out = pThresh->output.pData[0];

                    for(i=0; i<pThresh->input.height; i++)
                    {
                        IMG_thr_gt2thr_16((const unsigned short*)in,
                                            (unsigned short*)out,
                                            (short)(pThresh->input.width),
                                            1,
                                            (unsigned short)(pThresh->thresh));
                        in += pThresh->input.y_stride;
                        out += pThresh->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_THR_LE2MIN_8:
                case DVP_KN_IMG_THR_LE2MIN_8:
                {
                    DVP_Threshold_t *pThresh = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    DVP_U08* in  = pThresh->input.pData[0];
                    DVP_U08* out = pThresh->output.pData[0];

                    for(i=0; i<pThresh->input.height; i++)
                    {
                        IMG_thr_le2min_8(in,
                                            out,
                                            (short)(pThresh->input.width),
                                            1,
                                            (unsigned char)(pThresh->thresh));
                        in += pThresh->input.y_stride;
                        out += pThresh->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_THR_LE2MIN_16:
                case DVP_KN_IMG_THR_LE2MIN_16:
                {
                    DVP_Threshold_t *pThresh = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    DVP_U08* in  = pThresh->input.pData[0];
                    DVP_U08* out = pThresh->output.pData[0];

                    for(i=0; i<pThresh->input.height; i++)
                    {
                        IMG_thr_le2min_16((const unsigned short*)in,
                                            (unsigned short*)out,
                                            (short)(pThresh->input.width),
                                            1,
                                            (unsigned short)(pThresh->thresh));
                        in += pThresh->input.y_stride;
                        out += pThresh->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_THR_LE2THR_8:
                case DVP_KN_IMG_THR_LE2THR_8:
                {
                    DVP_Threshold_t *pThresh = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    DVP_U08* in  = pThresh->input.pData[0];
                    DVP_U08* out = pThresh->output.pData[0];

                    for(i=0; i<pThresh->input.height; i++)
                    {
                        IMG_thr_le2thr_8(in,
                                            out,
                                            (short)(pThresh->input.width),
                                            1,
                                            (unsigned char)(pThresh->thresh));
                        in += pThresh->input.y_stride;
                        out += pThresh->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_THR_LE2THR_16:
                case DVP_KN_IMG_THR_LE2THR_16:
                {
                    DVP_Threshold_t *pThresh = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    DVP_U08* in  = pThresh->input.pData[0];
                    DVP_U08* out = pThresh->output.pData[0];

                    for(i=0; i<pThresh->input.height; i++)
                    {
                        IMG_thr_le2thr_16((const unsigned short*)in,
                                            (unsigned short*)out,
                                            (short)(pThresh->input.width),
                                            1,
                                            (unsigned short)(pThresh->thresh));
                        in += pThresh->input.y_stride;
                        out += pThresh->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_IMG_BOUNDARY_8: // Assumes image is in 1D buffer
                {
                    DVP_Boundary_t *pBoundary = dvp_knode_to(&pSubNodes[n], DVP_Boundary_t);

                    IMG_boundary_8((const unsigned char*)pBoundary->input.pData[0],
                             (int)pBoundary->input.height,
                             (int)pBoundary->input.width,
                             (int*)pBoundary->outCoord.pData,
                             (int*)pBoundary->outGray.pData);
                    break;
                }
                case DVP_KN_IMG_BOUNDARY_16s: // Assumes image is in 1D buffer
                {
                    DVP_Boundary_t *pBoundary = dvp_knode_to(&pSubNodes[n], DVP_Boundary_t);

                    IMG_boundary_16s((const short*)pBoundary->input.pData[0],
                                     (int)pBoundary->input.height,
                                     (int)pBoundary->input.width,
                                     (unsigned int*)pBoundary->outCoord.pData,
                                     (short*)pBoundary->outGray.pData);
                    break;
                }
                case DVP_KN_IMG_PERIMETER_8: // Assumes image is in 1D buffer
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    DVP_U08* in  = pT->input.pData[0] + pT->input.y_stride; // Start at second line
                    DVP_U08* out = pT->output.pData[0];

                    for(i=0; i<pT->input.height-2; i++)
                    {
                        IMG_perimeter_8 ((const unsigned char*)in,
                                         (int)pT->input.width,
                                         (unsigned char*)out);
                        in += pT->input.y_stride;
                        out += pT->output.y_stride;
                    }
                    break;
                }
                case DVP_KN_IMG_PERIMETER_16: // Assumes image is in 1D buffer
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    DVP_U16* in  = (DVP_U16*)(pT->input.pData[0] + pT->input.y_stride); // Start at second line
                    DVP_U16* out = (DVP_U16*)pT->output.pData[0];

                    for(i=0; i<pT->input.height-2; i++)
                    {
                        IMG_perimeter_16 ((const unsigned short*)in,
                                         (int)pT->input.width,
                                         (unsigned short*)out);
                        in += pT->input.y_stride/2;
                        out += pT->output.y_stride/2;
                    }
                    break;
                }
                case DVP_KN_IMG_ERRDIFF_BIN_8:
                {
                    DVP_Threshold_t *pED =  dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    DVP_U08* in  = pED->input.pData[0];
                    int tmp_height = 0;
                    DVP_U32 inStride = pED->input.y_stride;
                    if(pED->input.y_stride > (int)pED->input.width)
                        tmp_height = pED->input.height;
                    else
                        tmp_height = 1;

                    for(i=0; i<(pED->input.height/tmp_height); i++)
                    {
                        IMG_errdif_bin_8(in,
                                      (int)pED->input.width,
                                      tmp_height,
                                      (short*)pED->output.pData[0],
                                      pED->thresh);
                        in += inStride;
                    }
                    break;
                }
                case DVP_KN_IMG_ERRDIFF_BIN_16:
                {
                    DVP_Threshold_t *pED =  dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                    DVP_U08* in  = pED->input.pData[0];
                    int tmp_height = 0;
                    DVP_U32 inStride = pED->input.y_stride;
                    if(pED->input.y_stride > (int)(2*pED->input.width))
                        tmp_height = pED->input.height;
                    else
                        tmp_height = 1;

                    for(i=0; i<(pED->input.height/tmp_height); i++)
                    {
                        IMG_errdif_bin_16((unsigned short*)in,
                                      (int)pED->input.width,
                                      tmp_height,
                                      (short*)pED->output.pData[0],
                                      pED->thresh);
                        in += inStride;
                    }
                    break;
                }
                case DVP_KN_IMG_FDCT_8x8:
                {
                    DVP_Transform_t *pImg =  dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    int num_fdcts = (pImg->input.width/8)*(pImg->input.height/8);
                    if(inStride == 2*pImg->input.width)
                    {
                        IMG_fdct_8x8((short*)in, num_fdcts);
                    }
                    break;
                }
                case DVP_KN_IMG_IDCT_8x8_12Q4:
                {
                    DVP_Transform_t *pImg =  dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    DVP_U08* in  = pImg->input.pData[0];
                    DVP_U32 inStride = pImg->input.y_stride;
                    int num_fdcts = (pImg->input.width/8)*(pImg->input.height/8);
                    if(inStride == 2*pImg->input.width)
                    {
                        IMG_idct_8x8_12q4((short*)in, num_fdcts);
                    }
                    break;
                }
                case DVP_KN_IMG_QUANTIZE_16:
                {
                    DVP_ImageQuantization_t *pQnt =  dvp_knode_to(&pSubNodes[n], DVP_ImageQuantization_t);
                    DVP_U08* data  = pQnt->input.pData[0];
                    DVP_U32 inStride = pQnt->input.y_stride;
                    DVP_U32 n_blocks = 0;
                    DVP_U32 loop_cnt = 0;
                    DVP_U32 i;
                    if(inStride == 2*pQnt->input.width)
                    {
                        n_blocks = (pQnt->input.width*pQnt->input.height)/pQnt->block_size;
                        loop_cnt = 1;
                    }
                    else
                    {
                        n_blocks = pQnt->input.width/pQnt->block_size;
                        loop_cnt = pQnt->input.height;
                    }
                    for(i=0; i<loop_cnt; i++)
                    {
                        IMG_quantize((short*)data,
                                     n_blocks,
                                     pQnt->block_size,
                                     (short*)pQnt->recip_tbl.pData,
                                     pQnt->q_pt);
                        data += inStride;
                    }
                    break;
                }
#endif
                default:
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: DVP KGM CPU Unknown Kernel %u\n",kernel);
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                    processed--;
                    break;
                }
            }
            processed++;
            DVP_PerformanceStop(pPerf);
        }
        DVP_PRINT(DVP_ZONE_KGM, "DVP KGM CPU: Processed %u nodes!\n", processed);
    }
    return processed;
}

static thread_ret_t DVP_KernelGraphManagerThread_CPU(void *arg __attribute__((unused)))
{
    DVP_KGM_Thread_t kgmt;
    DVP_S32 processed = 0;

    thread_nextaffinity();

    while (queue_read(workqueue, true_e, &kgmt) == true_e)
    {
        DVP_KernelNode_t *pSubNodes = kgmt.pSubNodes;
        DVP_U32 startNode = kgmt.startNode;
        DVP_U32 numNodes = kgmt.numNodes;
        kgmt.numNodesExecuted = 0;
        processed = 0;

        DVP_PRINT(DVP_ZONE_KGM, "DVP KGM CPU Thread Read a Work Item! %p[%d] (%p) for %d nodes\n",
            pSubNodes, startNode, &pSubNodes[startNode], numNodes);

        processed = DVP_KernelGraphManager_CPU(pSubNodes, startNode, numNodes);

        kgmt.numNodesExecuted = processed;
        queue_write(retqueue, true_e, &kgmt);
    }
    DVP_PRINT(DVP_ZONE_KGM, "DVP KGM CPU: Worker Thread Exitting!\n");
    thread_exit(0);
}

MODULE_EXPORT void DVP_KernelGraphManagerRestart(void *arg  __attribute__((unused)))
{
    // do nothing, since this should never be called.
}

MODULE_EXPORT DVP_BOOL DVP_KernelGraphManagerDeinit(void)
{
    DVP_U32 i;
    queue_pop(workqueue);
    queue_pop(retqueue);
    for (i = 0; i < dimof(workers); i++)
        thread_join(workers[i]);
    queue_destroy(workqueue);
    queue_destroy(retqueue);
    return DVP_TRUE;
}

MODULE_EXPORT DVP_BOOL DVP_KernelGraphManagerInit(DVP_RPC_t *pRPC __attribute__ ((unused)),
                                    DVP_RPC_Core_t *pCore __attribute__ ((unused)))
{
    DVP_U32 i = 0;
    workqueue = queue_create(10, sizeof(DVP_KGM_Thread_t));
    retqueue = queue_create(10, sizeof(DVP_KGM_Thread_t));
    for (i = 0; i < dimof(workers); i++)
        workers[i] = thread_create(DVP_KernelGraphManagerThread_CPU, NULL);
    return DVP_TRUE;
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
                {
                    DVP_PRINT(DVP_ZONE_WARNING, "Failed to read from return queue, trying again!\n");
                }
            } while (1);
        }
        return 0;
    }
    else
    {
        return DVP_KernelGraphManager_CPU(pSubNodes, startNode, numNodes);
    }
}

MODULE_EXPORT DVP_U32 DVP_KernelGraphManagerVerify(DVP_KernelNode_t *pSubNodes,
                                                   DVP_U32 startNode,
                                                   DVP_U32 numNodes)
{
    DVP_U32 n;
    DVP_U32 verified = 0;
    for (n = startNode; n < startNode + numNodes; n++)
    {
        // assume it will pass then set errors if detected.
        pSubNodes[n].header.error = DVP_SUCCESS;

        // check each supported kernel
        switch (pSubNodes[n].header.kernel)
        {
            // for Transforms, check WxH and NULL base pointers
            case DVP_KN_COPY:
            {
                DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, &pT->output.color, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, &pT->input.color, 1) == DVP_FALSE ||
                    pT->input.width > pT->output.width ||
                    pT->input.height > pT->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_XYXY_TO_Y800:
            {
                fourcc_t valid_colors[] = {FOURCC_UYVY, FOURCC_VYUY, FOURCC_Y800};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 2, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YXYX_TO_Y800:
#if defined(DVP_USE_VLIB)
            case DVP_KN_VLIB_YXYX_TO_Y800:
#endif
            {
                fourcc_t valid_colors[] = {FOURCC_YUY2, FOURCC_YVYU, FOURCC_Y800};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 2, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_Y800_TO_XYXY:
            {
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_UYVY, FOURCC_VYUY};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 2) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_RGBp:
#if defined(DVP_USE_VLIB)
            case DVP_KN_VLIB_UYVY_TO_RGBp:
#endif
            {
                fourcc_t valid_colors[] = {FOURCC_UYVY, FOURCC_RGBP};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_BGR:
            {
                fourcc_t valid_colors[] = {FOURCC_UYVY, FOURCC_BGR};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_YUV420p:
#if defined(DVP_USE_VLIB)
            case DVP_KN_VLIB_UYVY_TO_YUV420p:
#endif
            {
                fourcc_t valid_colors[] = {FOURCC_UYVY, FOURCC_IYUV, FOURCC_YV12};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 2) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_YUV422p:
#if defined(DVP_USE_VLIB)
            case DVP_KN_VLIB_UYVY_TO_YUV422p:
#endif
            {
                fourcc_t valid_colors[] = {FOURCC_UYVY, FOURCC_YU16, FOURCC_YV16};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 2) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_YUV444p:
#if defined(DVP_USE_VLIB)
            case DVP_KN_VLIB_UYVY_TO_YUV444p:
#endif
            {
                fourcc_t valid_colors[] = {FOURCC_UYVY, FOURCC_YU24, FOURCC_YV24};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 2) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YUV420p_TO_RGBp:
#if defined(DVP_USE_YUV)
            case DVP_KN_YUV_IYUV_TO_RGBp:
#endif
            {
                fourcc_t valid_colors[] = {FOURCC_IYUV, FOURCC_YV12, FOURCC_RGBP};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 2, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YUV420p_TO_UYVY:
            {
                fourcc_t valid_colors[] = {FOURCC_IYUV, FOURCC_YV12, FOURCC_UYVY};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 2, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YUV422p_TO_UYVY:
#if defined(DVP_USE_VLIB)
            case DVP_KN_VLIB_YUV422p_TO_UYVY:
#endif
            {
                fourcc_t valid_colors[] = {FOURCC_YU16, FOURCC_YV16, FOURCC_UYVY};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 2, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YUV444p_TO_RGBp:
#if defined(DVP_USE_YUV)
            case DVP_KN_YUV_YUV444_TO_RGBp:
#endif
            {
                fourcc_t valid_colors[] = {FOURCC_YU24, FOURCC_YV24, FOURCC_RGBP};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 2, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YUV444p_TO_UYVY:
            {
                fourcc_t valid_colors[] = {FOURCC_YU24, FOURCC_YV24, FOURCC_UYVY};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 2, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_NV12_TO_UYVY:
            {
                fourcc_t valid_colors[] = {FOURCC_NV12, FOURCC_UYVY};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_NV12_TO_YUV444p:
            {
                DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_NV12, FOURCC_YU24, FOURCC_YV24};
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, &valid_colors[1], 2) == DVP_FALSE ||
                    pT->input.width > pT->output.width*2 ||
                    pT->input.height > pT->output.height*2)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_BGR3_TO_UYVY:
#if defined(DVP_USE_YUV)
            case DVP_KN_YUV_BGR_TO_UYVY:
#endif
            {
                fourcc_t valid_colors[] = {FOURCC_BGR, FOURCC_UYVY};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_BGR3_TO_IYUV:
#if defined(DVP_USE_YUV)
            case DVP_KN_YUV_BGR_TO_IYUV:
#endif
            {
                fourcc_t valid_colors[] = {FOURCC_BGR, FOURCC_IYUV};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_BGR3_TO_NV12:
            {
                fourcc_t valid_colors[] = {FOURCC_BGR, FOURCC_NV12};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#ifdef DVP_USE_YUV
            case DVP_KN_YUV_ARGB_TO_UYVY:
            {
                fourcc_t valid_colors[] = {FOURCC_ARGB, FOURCC_UYVY};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YUV_IYUV_TO_BGR:
            {
                fourcc_t valid_colors[] = {FOURCC_IYUV, FOURCC_YV12, FOURCC_BGR};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 2, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YUV_IYUV_TO_RGB565:
            {
                fourcc_t valid_colors[] = {FOURCC_IYUV, FOURCC_YV12, FOURCC_RGB565};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 2, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            // case DVP_KN_YUV_NV12_TO_YU24_HALF_SCALE:
            // case DVP_KN_YUV_UYVY_HALF_SCALE:
            // case DVP_KN_YUV_UYVY_QTR_SCALE:
            // case DVP_KN_YUV_UYVY_ROTATE_CCW_90:
            // case DVP_KN_YUV_UYVY_ROTATE_CW_90:
            case DVP_KN_YUV_UYVY_TO_BGR:
            case DVP_KN_YUV_UYVY_TO_IYUV:
            case DVP_KN_YUV_UYVY_TO_RGBp:
            // case DVP_KN_YUV_UYVY_TO_RGBp_Y800_YU24:
            case DVP_KN_YUV_UYVY_TO_YUV444p:
            case DVP_KN_YUV_XYXY_TO_Y800:
            // case DVP_KN_YUV_Y800_ROTATE_CCW_90:
            // case DVP_KN_YUV_Y800_ROTATE_CW_90:
            case DVP_KN_YUV_Y800_TO_XYXY:
            case DVP_KN_YUV_YXYX_TO_Y800:
#endif
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_INIT_MEAN_LUMA_S16:
            case DVP_KN_VLIB_INIT_MEAN_LUMA_S32:
#endif
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_YUV422p_TO_RGB565:
#endif
            {
                DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    pT->input.width > pT->output.width ||
                    pT->input.height > pT->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_SOBEL_8:
            case DVP_KN_PREWITT_8:
            case DVP_KN_SCHARR_8:
            case DVP_KN_KROON_8:
            case DVP_KN_SOBEL_3x3_8:
            case DVP_KN_SOBEL_3x3_8s:
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_SOBEL_3x3_8:
            case DVP_KN_IMG_SOBEL_5x5_8:
            case DVP_KN_IMG_SOBEL_7x7_8:
            case DVP_KN_IMG_SOBEL_3x3_8s:
            case DVP_KN_IMG_SOBEL_5x5_8s:
            case DVP_KN_IMG_SOBEL_7x7_8s:
            case DVP_KN_IMG_MEDIAN_3x3_8:
            case DVP_KN_IMG_PERIMETER_8:
#endif
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_IMAGE_PYRAMID_8:
#endif
            {
                DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pT->input.width > pT->output.width ||
                    pT->input.height > pT->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_SOBEL_3x3_16:
            case DVP_KN_SOBEL_3x3_16s:
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_SOBEL_3x3_16:
            case DVP_KN_IMG_SOBEL_5x5_16:
            case DVP_KN_IMG_SOBEL_7x7_16:
            case DVP_KN_IMG_SOBEL_3x3_16s:
            case DVP_KN_IMG_SOBEL_5x5_16s:
            case DVP_KN_IMG_SOBEL_7x7_16s:
            case DVP_KN_IMG_MEDIAN_3x3_16s:
            case DVP_KN_IMG_PERIMETER_16:
            case DVP_KN_IMG_FDCT_8x8:
            case DVP_KN_IMG_IDCT_8x8_12Q4:
#endif
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_IMAGE_PYRAMID_16:
#endif
            {
                DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pT->input.width > pT->output.width ||
                    pT->input.height > pT->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#if defined(DVP_USE_IMGLIB)
            case DVP_KN_IMG_PIX_EXPAND:
            {
                DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    pT->input.width > pT->output.width ||
                    pT->input.height > pT->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
#if defined(DVP_USE_VLIB)
            case DVP_KN_VLIB_EXTRACT_BACK_8_16:
#endif
#if defined(DVP_USE_IMGLIB)
            case DVP_KN_IMG_PIX_SAT:
#endif
#if defined(DVP_USE_IMGLIB) || defined(DVP_USE_VLIB)
            {
                DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_Y16, FOURCC_Y800};
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    pT->input.width > pT->output.width ||
                    pT->input.height > pT->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
            case DVP_KN_THRESHOLD:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pIO->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pIO->input.height > pIO->output.height ||
                    pIO->input.width > pIO->output.width)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_XSTRIDE_CONVERT:
            case DVP_KN_XSTRIDE_SHIFT:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    pIO->input.height > pIO->output.height ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.x_stride == pIO->output.x_stride ||
                    pIO->input.x_stride > 2)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMAGE_DEBUG:
            {
                DVP_ImageDebug_t* pImg = dvp_knode_to(&pSubNodes[n],DVP_ImageDebug_t);
                if (DVP_Image_Validate(&pImg->image, 1, 1, 1, 1, NULL, 0) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_BUFFER_DEBUG:
            {
                DVP_BufferDebug_t* pBuf = dvp_knode_to(&pSubNodes[n],DVP_BufferDebug_t);
                if (DVP_Buffer_Validate(&pBuf->buffer) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }


            /*
            // these optimizations are never directly called
            case DVP_KN_OPT_UYVY_TO_LUMA_RGBp_YUV444p:
            case DVP_KN_OPT_UYVY_TO_LUMA_YUV444p_RGBp:
            case DVP_KN_OPT_UYVY_TO_RGBp_LUMA_YUV444p:
            case DVP_KN_OPT_UYVY_TO_RGBp_YUV444p_LUMA:
            case DVP_KN_OPT_UYVY_TO_YUV444p_LUMA_RGBp:
            case DVP_KN_OPT_UYVY_TO_YUV444p_RGBp_LUMA:
            */
            case DVP_KN_DILATE_CROSS:
            case DVP_KN_DILATE_MASK:
            case DVP_KN_DILATE_SQUARE:
            case DVP_KN_ERODE_CROSS:
            case DVP_KN_ERODE_MASK:
            case DVP_KN_ERODE_SQUARE:
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_DILATE_CROSS:
            case DVP_KN_VLIB_DILATE_MASK:
            case DVP_KN_VLIB_DILATE_SQUARE:
            case DVP_KN_VLIB_ERODE_CROSS:
            case DVP_KN_VLIB_ERODE_MASK:
            case DVP_KN_VLIB_ERODE_SQUARE:
            case DVP_KN_VLIB_ERODE_SINGLEPIXEL:
#endif
            {
                DVP_Morphology_t *pD = dvp_knode_to(&pSubNodes[n], DVP_Morphology_t);
                if (DVP_Image_Validate(&pD->input, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pD->output, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    pD->input.width != pD->output.width ||
                    pD->input.height != pD->output.height ||
                    pD->input.color != FOURCC_Y800 ||
                    pD->output.color != FOURCC_Y800)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_CANNY_2D_GRADIENT:
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_CANNY_2D_GRADIENT:
#endif
            {
                DVP_Canny2dGradient_t *pD = dvp_knode_to(&pSubNodes[n], DVP_Canny2dGradient_t);
                if (DVP_Image_Validate(&pD->input, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pD->outGradX, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pD->outGradY, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pD->outMag, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    pD->input.width != pD->outGradX.width ||
                    pD->input.width != pD->outGradY.width ||
                    pD->input.width != pD->outMag.width ||
                    pD->input.height != pD->outGradX.height ||
                    pD->input.height != pD->outGradY.height ||
                    pD->input.height != pD->outMag.height ||
                    pD->input.color != FOURCC_Y800 ||
                    pD->outGradX.color != FOURCC_Y16 ||
                    pD->outGradY.color != FOURCC_Y16 ||
                    pD->outMag.color != FOURCC_Y16)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_CANNY_HYST_THRESHHOLD:
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_CANNY_HYST_THRESHHOLD:
#endif
            {
                /*! \todo add check here */
                break;
            }
            case DVP_KN_CANNY_NONMAX_SUPPRESSION:
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_CANNY_NONMAX_SUPPRESSION:
#endif
            {
                /*! \todo add check here */
                break;
            }
            case DVP_KN_CONV_3x3:
            case DVP_KN_CONV_5x5:
            case DVP_KN_CONV_7x7:
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_CONV_3x3:
            case DVP_KN_IMG_CONV_5x5:
            case DVP_KN_IMG_CONV_7x7:
            case DVP_KN_IMG_CONV_11x11:
#endif
            case DVP_KN_CANNY_IMAGE_SMOOTHING:
            {
                DVP_ImageConvolution_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->mask, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    /*! \todo what are the valid shiftMask ranges */
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_CONV_3x3_I16s_C16:
            case DVP_KN_IMG_CONV_7x7_I16s_C16:
            case DVP_KN_IMG_CONV_11x11_I16s_C16:
            {
                DVP_ImageConvolution_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->mask, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CONV_5x5_I8_C16:
            case DVP_KN_IMG_CONV_7x7_I8_C16:
            {
                DVP_ImageConvolution_with_buffer_t *pImgb = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_with_buffer_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pImgb->input, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImgb->output, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImgb->mask, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    pImgb->input.width > pImgb->output.width ||
                    pImgb->input.height > pImgb->output.height ||
                    pImgb->scratch.pData == NULL)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CONV_5x5_I16s_C16:
            {
                DVP_ImageConvolution_with_buffer_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageConvolution_with_buffer_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->mask, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height ||
                    pImg->scratch.pData == NULL)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_CORR_3x3:
            {
                DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    pImg->input.width != pImg->output.width ||
                    pImg->input.height != pImg->output.height ||
                    pImg->input.color != FOURCC_Y800 ||
                    pImg->output.color != FOURCC_RGBA ||
                    pImg->mask.color != FOURCC_Y800 ||
                    pImg->shift > 8)
                    /*! \todo check other correlation parameters */
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CORR_3x3_I16s_C16s:
            case DVP_KN_IMG_CORR_5x5_I16s_C16s:
            case DVP_KN_IMG_CORR_11x11_I16s_C16s:
            {
                DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    pImg->input.width != pImg->output.width ||
                    pImg->input.height != pImg->output.height ||
                    pImg->input.color != FOURCC_Y16 ||
                    pImg->output.color != FOURCC_RGBA ||
                    pImg->mask.color != FOURCC_Y16 ||
                    pImg->shift > 16)
                    /*! \todo check other correlation parameters */
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CORR_3x3_I8_C16s:
            case DVP_KN_IMG_CORR_11x11_I8_C16s:
            {
                DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    pImg->input.width != pImg->output.width ||
                    pImg->input.height != pImg->output.height ||
                    pImg->input.color != FOURCC_Y800 ||
                    pImg->output.color != FOURCC_RGBA ||
                    pImg->mask.color != FOURCC_Y16 ||
                    pImg->shift > 16)
                    /*! \todo check other correlation parameters */
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CORR_GEN_I16s_C16s:
            {
                DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->mask, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height ||
                    pImg->m != pImg->mask.width)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
               break;
            }
            case DVP_KN_IMG_CORR_GEN_IQ:
            {
                DVP_ImageCorrelation_t *pImg = dvp_knode_to(&pSubNodes[n], DVP_ImageCorrelation_t);
                fourcc_t valid_colors[] = {FOURCC_Y32, FOURCC_Y16};
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->mask, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
            case DVP_KN_IIR_HORZ:
            case DVP_KN_IIR_VERT:
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_IIR_HORZ:
            case DVP_KN_VLIB_IIR_HORZ_16:
            case DVP_KN_VLIB_IIR_VERT:
            case DVP_KN_VLIB_IIR_VERT_16:
#endif
            {
                DVP_IIR_t *iir = dvp_knode_to(&pSubNodes[n], DVP_IIR_t);
                if (DVP_Image_Validate(&iir->input, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&iir->output, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    /*! \todo add checking for bounds */
                    iir->weight == 0)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_INTEGRAL_IMAGE_8:
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_INTEGRAL_IMAGE_8:
#endif
            {
                DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    pT->input.width != pT->output.width ||
                    pT->input.height != pT->output.height ||
                    pT->input.color != FOURCC_Y800 ||
                    pT->output.color != FOURCC_Y32)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_INTEGRAL_IMAGE_16:
            {
                DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, NULL, 0) == DVP_FALSE ||
                    pT->input.width != pT->output.width ||
                    pT->input.height != pT->output.height ||
                    pT->input.color != FOURCC_Y16 ||
                    pT->output.color != FOURCC_Y32)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
            case DVP_KN_NONMAXSUPPRESS_3x3_S16:
            case DVP_KN_NONMAXSUPPRESS_5x5_S16:
            case DVP_KN_NONMAXSUPPRESS_7x7_S16:
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_NONMAXSUPPRESS_3x3_S16:
            case DVP_KN_VLIB_NONMAXSUPPRESS_5x5_S16:
            case DVP_KN_VLIB_NONMAXSUPPRESS_7x7_S16:
#endif
            {
                DVP_Threshold_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                /*! \todo any threshold value is ok? */
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    pT->input.width > pT->output.width ||
                    pT->input.height > pT->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_THR_GT2MAX_8:
            case DVP_KN_THR_GT2THR_8:
            case DVP_KN_THR_LE2MIN_8:
            case DVP_KN_THR_LE2THR_8:
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_THR_GT2MAX_8:
            case DVP_KN_IMG_THR_GT2THR_8:
            case DVP_KN_IMG_THR_LE2MIN_8:
            case DVP_KN_IMG_THR_LE2THR_8:
#endif
            {
                DVP_Threshold_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pT->input.width > pT->output.width ||
                    pT->input.height > pT->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_ERRDIFF_BIN_8:
            {
                DVP_Threshold_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    pT->input.width > pT->output.width ||
                    pT->input.height > pT->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
            case DVP_KN_THR_GT2MAX_16:
            case DVP_KN_THR_GT2THR_16:
            case DVP_KN_THR_LE2MIN_16:
            case DVP_KN_THR_LE2THR_16:
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_THR_GT2MAX_16:
            case DVP_KN_IMG_THR_GT2THR_16:
            case DVP_KN_IMG_THR_LE2MIN_16:
            case DVP_KN_IMG_THR_LE2THR_16:
            case DVP_KN_IMG_ERRDIFF_BIN_16:
#endif
            {
                DVP_Threshold_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Threshold_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pT->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pT->input.width > pT->output.width ||
                    pT->input.height > pT->output.height)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_NORMALFLOW_16:
            {
                DVP_NormalFlow_t *pN = dvp_knode_to(&pSubNodes[n], DVP_NormalFlow_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pN->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->x, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->y, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->u, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->v, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->lut, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->mag, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            /// @todo case DVP_KN_VLIB_KALMAN_2x4:
            /// @todo case DVP_KN_VLIB_KALMAN_4x6:
            case DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_16:
            case DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_3D:
            {
                DVP_Simplex_t *pSmx = dvp_knode_to(&pSubNodes[n], DVP_Simplex_t);
                if (DVP_Buffer_Validate(&pSmx->start) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->step) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->stop) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[0]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[1]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[2]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[3]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[4]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[5]) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            /// @todo case DVP_KN_VLIB_LEGENDRE_MOMENTS:
            case DVP_KN_VLIB_INIT_HISTOGRAM_8:
            case DVP_KN_VLIB_HISTOGRAM_8:
            case DVP_KN_VLIB_WEIGHTED_HISTOGRAM_8:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pH->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->edges) == DVP_FALSE ||
                    //DVP_Buffer_Validate(&pH->numBinsArray) == DVP_FALSE ||
                    //DVP_Buffer_Validate(&pH->binWeights) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->hOut) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->histArray) == DVP_FALSE)
                    /*! \todo check scratch, h[3], numBinArray, binWeights */
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_INIT_HISTOGRAM_16:
            case DVP_KN_VLIB_HISTOGRAM_16:
            case DVP_KN_VLIB_WEIGHTED_HISTOGRAM_16:
            case DVP_KN_VLIB_HISTOGRAM_ND_16:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pH->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->edges) == DVP_FALSE ||
                    //DVP_Buffer_Validate(&pH->numBinsArray) == DVP_FALSE ||
                    //DVP_Buffer_Validate(&pH->binWeights) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->hOut) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->histArray) == DVP_FALSE)
                    /*! \todo check scratch, h[3], numBinArray, binWeights */
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_NONMAXSUPPRESS_U32:
            {
                DVP_Nonmax_NxN_t *nonmax32 = dvp_knode_to(&pSubNodes[n], DVP_Nonmax_NxN_t);
                fourcc_t valid_colors[] = {FOURCC_Y32};
                if (DVP_Image_Validate(&nonmax32->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Buffer_Validate(&nonmax32->pixIndex) == DVP_FALSE ||
                    DVP_Buffer_Validate(&nonmax32->scratch) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

#if 0 // @todo Must test and enable these features
            case DVP_KN_VLIB_HISTOGRAM_ND_16:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                break;
            }
#endif
            case DVP_KN_VLIB_BHATTACHAYA_DISTANCE:
            case DVP_KN_VLIB_L1DISTANCE:
            {
                DVP_Distance_t *pD = dvp_knode_to(&pSubNodes[n], DVP_Distance_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pD->X, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pD->Y, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_TRACK_FEATURES_LUCAS_7x7:
            {
                DVP_TrackFeatures_t *pTF = dvp_knode_to(&pSubNodes[n], DVP_TrackFeatures_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pTF->im1, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pTF->im2, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pTF->gradx, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pTF->grady, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pTF->x) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pTF->y) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pTF->outx) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pTF->outy) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#if defined(VLIB_DISPARITY_FIXED)
            // these disparity calculations seem to be broken, we'll reenable once they are fixed.
            case DVP_KN_VLIB_DISPARITY_SAD8:
            {
                DVP_Disparity_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Disparity_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pT->left, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->right, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->cost, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->mincost, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->disparity, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_DISPARITY_SAD16:
            {
                DVP_Disparity_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Disparity_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pT->left, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->right, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->cost, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->mincost, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->disparity, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif // VLIB_DISPARITY_FIXED
            case DVP_KN_VLIB_EWR_MEAN_S16:
            {
                DVP_EMean_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_EMean_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->mean, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->mask, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->data, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    pParam->mask.width != pParam->data.width/8)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_EWR_MEAN_S32:
            {
                DVP_EMean_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_EMean_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y32};
                if (DVP_Image_Validate(&pParam->mean, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->mask, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->data, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    pParam->mask.width != pParam->data.width/8)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_EWR_VAR_S16:
            {
                DVP_EMean_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_EMean_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->var, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->mean, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->mask, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->data, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    pParam->mask.width != pParam->data.width/8)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_EWR_VAR_S32:
            {
                DVP_EMean_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_EMean_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y32};
                if (DVP_Image_Validate(&pParam->var, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->mean, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->mask, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->data, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    pParam->mask.width != pParam->data.width/8)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_UWR_MEAN_S16:
            {
                DVP_UMean_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_UMean_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->old_mean, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->old_mask, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->old_data, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->new_mean, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->new_mask, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->new_data, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    pParam->old_mask.width != pParam->old_data.width/8 ||
                    pParam->new_mask.width != pParam->new_data.width/8)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_UWR_VAR_S16:
            {
                DVP_UMean_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_UMean_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->old_var, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->old_mean, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->old_mask, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->old_data, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->new_var, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE  ||
                    DVP_Image_Validate(&pParam->new_mean, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->new_mask, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->new_data, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    pParam->old_mask.width != pParam->old_data.width/8 ||
                    pParam->new_mask.width != pParam->new_data.width/8)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_SUB_BACK_S16:
            {
                DVP_SubBackground_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_SubBackground_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16, FOURCC_Y32};
                if (DVP_Image_Validate(&pParam->output32packed, 1, 1, 1, 1, &valid_colors[2], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->newestData, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->currentMean, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->currentVar, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_SUB_BACK_S32:
            {
                DVP_SubBackground_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_SubBackground_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16, FOURCC_Y32};
                if (DVP_Image_Validate(&pParam->output32packed, 1, 1, 1, 1, &valid_colors[2], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->newestData, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->currentMean, 1, 1, 1, 1, &valid_colors[2], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->currentVar, 1, 1, 1, 1, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_MOG_S16:
            {
                DVP_Mog_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_Mog_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16, FOURCC_Y32};
                if (DVP_Image_Validate(&pParam->image, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->wts, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->means, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->vars, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->scratch, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->compIndex, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->fgmask, 1, 1, 1, 1, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_MOG_S32:
            {
                DVP_Mog_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_Mog_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16, FOURCC_Y32};
                if (DVP_Image_Validate(&pParam->image, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->wts, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->means, 1, 1, 1, 1, &valid_colors[2], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->vars, 1, 1, 1, 1, &valid_colors[2], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->scratch, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->compIndex, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->fgmask, 1, 1, 1, 1, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_PACK_MASK_32:
            {
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_BIN1};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_UNPACK_MASK_32:
            {
                fourcc_t valid_colors[] = {FOURCC_BIN1, FOURCC_Y800};
                if (DVP_Transform_Check(&pSubNodes[n], valid_colors, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_CCL:
            {
                DVP_CCL_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_CCL_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE ||
                    pParam->input.width > pParam->output.width ||
                    pParam->input.height > pParam->output.height ||
                    (pParam->connected8Flag != DVP_CCL_4_CONNECTED &&
                     pParam->connected8Flag != DVP_CCL_8_CONNECTED))
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_HOUGH_LINE_FROM_LIST:
            {
                DVP_HoughLine_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_HoughLine_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->pOutHoughSpace, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->pEdgeMapList) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->pSIN) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->pCOS) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->ping) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->pong) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->pang) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->peng) == DVP_FALSE)
                    /*! \todo check more values from varaibles */
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_HARRIS_SCORE_7x7:
            {
                DVP_HarrisDSP_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_HarrisDSP_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->inGradX, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->inGradY, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->harrisScore, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_HARRIS_SCORE_7x7_U32:
            {
                DVP_HarrisDSP_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_HarrisDSP_t);
                fourcc_t valid_colors[] = {FOURCC_Y16, FOURCC_Y32};
                if (DVP_Image_Validate(&pParam->inGradX, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->inGradY, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->harrisScore, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_INIT_VAR_CONST_S16:
            {
                DVP_ConstVar_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_ConstVar_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->var, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_INIT_VAR_CONST_S32:
            {
                DVP_ConstVar_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_ConstVar_t);
                fourcc_t valid_colors[] = {FOURCC_Y32};
                if (DVP_Image_Validate(&pParam->var, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_8:
            case DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_16:
            {
                DVP_Gradient_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_Gradient_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->scratch, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_GRADIENT_H5x5_PYRAMID_8:
            case DVP_KN_VLIB_GRADIENT_V5x5_PYRAMID_8:
            {
                DVP_Gradient_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_Gradient_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->scratch, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

#endif /// DVP_USE_VLIB

#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_CLIPPING_16:
            {
                DVP_CannyHystThresholding_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_CannyHystThresholding_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->inMag, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_BOUNDARY_8:
            {
                DVP_Boundary_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_Boundary_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->outCoord) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->outGray) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_BOUNDARY_16s:
            {
                DVP_Boundary_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_Boundary_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->outCoord) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->outGray) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_HISTOGRAM_8:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pH->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->hOut) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->histArray) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_HISTOGRAM_16:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pSubNodes[n], DVP_Histogram_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pH->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->hOut) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->histArray) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif

#ifdef DVP_USE_VLIB
            case DVP_KN_VLIB_UYVY_TO_HSLp:
            case DVP_KN_VLIB_UYVY_TO_LABp:
#endif /// DVP_USE_VLIB
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_YC_DEMUX_BE16_8:
            case DVP_KN_IMG_YC_DEMUX_LE16_8:
#endif
#if defined(DVP_USE_VLIB) || defined(DVP_USE_IMGLIB)
            {
                DVP_Int2Pl_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_Int2Pl_t);
                fourcc_t valid_colors[] = {FOURCC_UYVY, FOURCC_Y800, FOURCC_Y16, FOURCC_RGBA};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output1, 1, 1, 1, 1, &valid_colors[1], 3) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output2, 1, 1, 1, 1, &valid_colors[1], 3) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output3, 1, 1, 1, 1, &valid_colors[1], 3) == DVP_FALSE)
                    /*! \todo should we be strictly pendantic here for matching all optional color choices? */
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif

#if defined(DVP_USE_ORB)
            case DVP_KN_ORB:
            {
                DVP_Orb_t * pParam = dvp_knode_to(&pSubNodes[n], DVP_Orb_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y16, FOURCC_Y32};
                if (DVP_Image_Validate(&pParam->inputImage, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->harrisImage, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->integralImage, 1, 1, 1, 1, &valid_colors[2], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif//DVP_USE_ORB
#if defined(DVP_USE_RVM)
            case DVP_KN_RVM:
            {
                DVP_Rvm_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_Rvm_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_UYVY};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pParam->input.color != pParam->output.color ||
                    DVP_Buffer_Validate(&pParam->lut) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif // DVP_USE_RVM
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_SAD_3x3:
            case DVP_KN_IMG_SAD_5x5:
            case DVP_KN_IMG_SAD_7x7:
            case DVP_KN_IMG_SAD_8x8:
            case DVP_KN_IMG_SAD_16x16:
            {
                DVP_SAD_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_SAD_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->refImg, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_MAD_8x8:
            case DVP_KN_IMG_MAD_16x16:
            {
                DVP_SAD_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_SAD_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_Y32};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output, 1, 1, 1, 1, &valid_colors[1], 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->refImg, 1, 1, 1, 1, &valid_colors[0], 1) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_QUANTIZE_16:
            {
                DVP_ImageQuantization_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_ImageQuantization_t);
                fourcc_t valid_colors[] = {FOURCC_Y16};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
            case DVP_KN_GAMMA:
            {
                DVP_Gamma_t *pParam = dvp_knode_to(&pSubNodes[n], DVP_Gamma_t);
                fourcc_t valid_colors[] = {FOURCC_Y800};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE)
                    pSubNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            /*! \todo add each kernel supported in the CPU manager to this list */

            case DVP_KN_NOOP:
                break;

            default:
                // only set unimplemented when all the kernels have been added.
                //pSubNodes[n].header.error = DVP_ERROR_NOT_IMPLEMENTED;
                break;
        }
        if (pSubNodes[n].header.error == DVP_SUCCESS)
        {
            DVP_PRINT(DVP_ZONE_KGM, "Node[%u] of %u passed verification! (%u)\n", n, numNodes, verified);
            verified++;
        }
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

static_module_table_t dvp_kgm_cpu_table = {
    "dvp_kgm_cpu",
    dimof(dvp_kgm_functions),
    dvp_kgm_functions
};
#endif

void gamma_apply(DVP_Image_t *input, DVP_Image_t *output, DVP_U08* gammaLut)
{
    DVP_U08 *in, *out;
    DVP_U32 width, height;
    uint32_t i,j,indx,start_in_pixel = 0, start_out_pixel = 0,cur_in_pixel, cur_out_pixel;

    if (gammaLut == NULL)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to apply gamma mapping: invalid LUT.\n");
        return;
    }

    if (input == NULL || output == NULL)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to apply gamma mapping: invalid input and/or output images.\n");
        return;
    }

    in = input->pData[0];
    out = output->pData[0];

    width = input->width;
    height = input->height;


    if (width != output->width || height != output->height)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to apply gamma mapping: input and output images must have the same dimensions.\n");
        return;
    }

    for (
        i = 0;
        i < height;
        i++, start_in_pixel += input->y_stride, start_out_pixel += output->y_stride
        )
    {
        cur_in_pixel = start_in_pixel;
        cur_out_pixel = start_out_pixel;
        for (
            j = 0;
            j < width;
            j++, cur_in_pixel += input->x_stride, cur_out_pixel += output->x_stride
            )
        {
            indx = in[cur_in_pixel];
            out[cur_out_pixel] = gammaLut[indx];
        }
    }
}
