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

#if defined(DVP_USE_VLIB)
#include <vlib/dvp_kl_vlib.h>
#endif

#if defined(DVP_USE_IMGLIB)
#include <imglib/dvp_kl_imglib.h>
#endif

#if defined(DVP_USE_TISMO)
#include <tismo/dvp_kl_tismo.h>
#endif

#if defined(DVP_USE_TISMOV02)
#include <tismov02/dvp_kl_tismov02.h>
#endif

#if defined(DVP_USE_RVM)
#include <rvm/dvp_kl_rvm.h>
#endif

#if defined(DVP_USE_ORB)
#include <orb/dvp_kl_orb.h>
#endif

#if defined(DVP_USE_DSPLIB)
#include <dsplib/dvp_kl_dsplib.h>
#endif

#if defined(DVP_USE_DEI)
#include <dei/dvp_kl_dei.h>
#endif

#define KGM_TAG "DVP KGM DSP"

#if (defined(DVP_USE_IMGLIB) || defined(DVP_USE_VLIB))
static dvp_image_shift_t dsp_shift3 = {
    -1, -1, 0, 2, 2, 0,
};
#endif

#if (defined(DVP_USE_IMGLIB) || defined(DVP_USE_VLIB))
static dvp_image_shift_t dsp_shift5 = {
    -2, -2, 0, 4, 4, 0,
};
#endif

#if (defined(DVP_USE_IMGLIB) /*|| defined(DVP_USE_VLIB)*/)
static dvp_image_shift_t dsp_shift7 = {
    -3, -3, 0, 6, 6, 0,
};
#endif

#if (defined(DVP_USE_IMGLIB) /*|| defined(DVP_USE_VLIB)*/)
static dvp_image_shift_t dsp_shift8 = {
    -4, -4, 0, 7, 7, 0,
};
#endif

#if (defined(DVP_USE_IMGLIB) /*|| defined(DVP_USE_VLIB)*/)
static dvp_image_shift_t dsp_shift11 = {
    -5, -5, 0, 10, 10, 0,
};
#endif

#if (defined(DVP_USE_IMGLIB) /*|| defined(DVP_USE_VLIB)*/)
static dvp_image_shift_t dsp_shift16 = {
    -8, -8, 0, 15, 15, 0,
};
#endif

#if (defined(DVP_USE_VLIB) /*|| defined(DVP_USE_IMGLIB) */)
static dvp_image_shift_t dsp_nonmax_shift3 = {
    -1, 1, 2, 2, 0, 0,
};
#endif

#if (defined(DVP_USE_VLIB) /*|| defined(DVP_USE_IMGLIB) */)
static dvp_image_shift_t dsp_nonmax_shift5 = {
    -2, 2, 4, 4, 0, 0,
};
#endif

#if (defined(DVP_USE_VLIB) /*|| defined(DVP_USE_IMGLIB) */)
static dvp_image_shift_t dsp_nonmax_shift7 = {
    -3, 3, 6, 6, 0, 0,
};
#endif

static DVP_CoreFunction_t remote_kernels[] = {
#ifdef DVP_USE_IPC
    {"c64t No operation",               DVP_KN_NOOP,                        0, NULL, NULL},
    {"c64t Echo",                       DVP_KN_COPY,                        0, NULL, NULL},
#ifdef DVP_USE_VLIB
    // name                             kernel                              priority,   load
    {"c64t EWRMeanS16",                 DVP_KN_VLIB_EWR_MEAN_S16,           0, NULL, NULL},
    {"c64t EWRVarS16",                  DVP_KN_VLIB_EWR_VAR_S16,            0, NULL, NULL},
    {"c64t EWRMeanS32",                 DVP_KN_VLIB_EWR_MEAN_S32,           0, NULL, NULL},
    {"c64t EWRVarS32",                  DVP_KN_VLIB_EWR_VAR_S32,            0, NULL, NULL},
    {"c64t UWRMeanS16",                 DVP_KN_VLIB_UWR_MEAN_S16,           0, NULL, NULL},
    {"c64t UWRVarS16",                  DVP_KN_VLIB_UWR_VAR_S16,            0, NULL, NULL},
    {"c64t BackSubS16",                 DVP_KN_VLIB_SUB_BACK_S16,           0, NULL, NULL},
    {"c64t BackSubS32",                 DVP_KN_VLIB_SUB_BACK_S32,           0, NULL, NULL},

    {"c64t MixofGaussS16",              DVP_KN_VLIB_MOG_S16,                0, NULL, NULL},
    {"c64t MixofGaussS32",              DVP_KN_VLIB_MOG_S32,                0, NULL, NULL},

    {"c64t ExtractBack16",              DVP_KN_VLIB_EXTRACT_BACK_8_16,      0, NULL, NULL},

    {"c64t PackMask32",                 DVP_KN_VLIB_PACK_MASK_32,           0, NULL, NULL},
    {"c64t UnPackMask32",               DVP_KN_VLIB_UNPACK_MASK_32,         0, NULL, NULL},

    // Generic Morph
    {"c64t DilateCross",                DVP_KN_DILATE_CROSS,                0, &dsp_shift3, NULL},
    {"c64t DilateMask",                 DVP_KN_DILATE_MASK,                 0, &dsp_shift3, NULL},
    {"c64t DilateSquare",               DVP_KN_DILATE_SQUARE,               0, &dsp_shift3, NULL},
    {"c64t ErodeCross",                 DVP_KN_ERODE_CROSS,                 0, &dsp_shift3, NULL},
    {"c64t ErodeMask",                  DVP_KN_ERODE_MASK,                  0, &dsp_shift3, NULL},
    {"c64t ErodeSquare",                DVP_KN_ERODE_SQUARE,                0, &dsp_shift3, NULL},

    {"c64t DilateCross",                DVP_KN_VLIB_DILATE_CROSS,           0, &dsp_shift3, NULL},
    {"c64t DilateMask",                 DVP_KN_VLIB_DILATE_MASK,            0, &dsp_shift3, NULL},
    {"c64t DilateSquare",               DVP_KN_VLIB_DILATE_SQUARE,          0, &dsp_shift3, NULL},
    {"c64t ErodeCross",                 DVP_KN_VLIB_ERODE_CROSS,            0, &dsp_shift3, NULL},
    {"c64t ErodeMask",                  DVP_KN_VLIB_ERODE_MASK,             0, &dsp_shift3, NULL},
    {"c64t ErodeSquare",                DVP_KN_VLIB_ERODE_SQUARE,           0, &dsp_shift3, NULL},
    {"c64t Erode1Pixel",                DVP_KN_VLIB_ERODE_SINGLEPIXEL,      0, NULL, NULL},

    {"c64t ConnectedComponent",         DVP_KN_VLIB_CCL,                    0, NULL, NULL},

    {"c64t Canny2DGradient",            DVP_KN_CANNY_2D_GRADIENT,           0, &dsp_shift3, NULL},
    {"c64t CannyNonmaxSupress",         DVP_KN_CANNY_NONMAX_SUPPRESSION,    0, &dsp_shift3, NULL},
//    {"c64t CannyHystThresh",            DVP_KN_CANNY_HYST_THRESHHOLD,       0, NULL, NULL},

    {"c64t Canny2DGradient",            DVP_KN_VLIB_CANNY_2D_GRADIENT,      0, &dsp_shift3, NULL},
    {"c64t CannyNonmaxSupress",         DVP_KN_VLIB_CANNY_NONMAX_SUPPRESSION, 0, &dsp_shift3, NULL},
//    {"c64t CannyHystThresh",            DVP_KN_VLIB_CANNY_HYST_THRESHHOLD,  0, NULL, NULL},
    {"c64t HystThresh",                 DVP_KN_VLIB_HYST_THRESHOLD,         0, NULL, NULL},

    {"c64t ImgPyramid8",                DVP_KN_VLIB_IMAGE_PYRAMID_8,        0, NULL, NULL},
    {"c64t ImgPyramid16",               DVP_KN_VLIB_IMAGE_PYRAMID_16,       0, NULL, NULL},

    {"c64t Gauss5x5Pyramid8",           DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_8,  0, &dsp_shift5, NULL},
    {"c64t Gauss5x5Pyramid16",          DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_16, 0, &dsp_shift5, NULL},

    {"c64t GradientH5x5Pyramid8",       DVP_KN_VLIB_GRADIENT_H5x5_PYRAMID_8, 0, &dsp_shift5, NULL},
    {"c64t GradientV5x5Pyramid8",       DVP_KN_VLIB_GRADIENT_V5x5_PYRAMID_8, 0, &dsp_shift5, NULL},

    {"c64t HarrisScore7x7",             DVP_KN_VLIB_HARRIS_SCORE_7x7,       0, NULL, NULL},
    {"c64t Track Features Lucas 7x7",   DVP_KN_VLIB_TRACK_FEATURES_LUCAS_7x7, 0, NULL, NULL},

    {"c64t IIRHorz",                    DVP_KN_IIR_HORZ,                    0, NULL, NULL},
    {"c64t IIRHorz",                    DVP_KN_VLIB_IIR_HORZ,               0, NULL, NULL},
    {"c64t IIRHorz16",                  DVP_KN_VLIB_IIR_HORZ_16,            0, NULL, NULL},
    {"c64t IIRVert",                    DVP_KN_IIR_VERT,                    0, NULL, NULL},
    {"c64t IIRVert",                    DVP_KN_VLIB_IIR_VERT,               0, NULL, NULL},
    {"c64t IIRVert16",                  DVP_KN_VLIB_IIR_VERT_16,            0, NULL, NULL},
    {"c64t IntegralImg8",               DVP_KN_INTEGRAL_IMAGE_8,            0, NULL, NULL},
    {"c64t IntegralImg8",               DVP_KN_VLIB_INTEGRAL_IMAGE_8,       0, NULL, NULL},

    {"c64t IntegralImg16",              DVP_KN_VLIB_INTEGRAL_IMAGE_16,      0, NULL, NULL},

    {"c64t HoughLine",                  DVP_KN_VLIB_HOUGH_LINE_FROM_LIST,   0, NULL, NULL},

    {"c64t Nonmaxsupress3x316",         DVP_KN_NONMAXSUPPRESS_3x3_S16,      0, &dsp_nonmax_shift3, NULL},
    {"c64t Nonmaxsupress5x516",         DVP_KN_NONMAXSUPPRESS_5x5_S16,      0, &dsp_nonmax_shift5, NULL},
    {"c64t Nonmaxsupress7x716",         DVP_KN_NONMAXSUPPRESS_7x7_S16,      0, &dsp_nonmax_shift7, NULL},

    {"c64t Nonmaxsupress3x316",         DVP_KN_VLIB_NONMAXSUPPRESS_3x3_S16, 0, &dsp_nonmax_shift3, NULL},
    {"c64t Nonmaxsupress5x516",         DVP_KN_VLIB_NONMAXSUPPRESS_5x5_S16, 0, &dsp_nonmax_shift5, NULL},
    {"c64t Nonmaxsupress7x716",         DVP_KN_VLIB_NONMAXSUPPRESS_7x7_S16, 0, &dsp_nonmax_shift7, NULL},

    {"c64t NormalFlow",                 DVP_KN_VLIB_NORMALFLOW_16,          0, NULL, NULL},
    {"c64t Kalman2x4",                  DVP_KN_VLIB_KALMAN_2x4,             0, NULL, NULL},
    {"c64t Kalman4x6",                  DVP_KN_VLIB_KALMAN_4x6,             0, NULL, NULL},
    {"c64t Nelder-MeadSimplex16",       DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_16, 0, NULL, NULL},
    {"c64t Nelder-MeanSimplex3D",       DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_3D, 0, NULL, NULL},
    {"c64t LegendreMoments",            DVP_KN_VLIB_LEGENDRE_MOMENTS,       0, NULL, NULL},
    {"c64t InitHistgram8",              DVP_KN_VLIB_INIT_HISTOGRAM_8,       0, NULL, NULL},
    {"c64t Histogram8",                 DVP_KN_VLIB_HISTOGRAM_8,            0, NULL, NULL},
    {"c64t WeightedHistogram8",         DVP_KN_VLIB_WEIGHTED_HISTOGRAM_8,   0, NULL, NULL},
    {"c64t InitHistogram16",            DVP_KN_VLIB_INIT_HISTOGRAM_16,      0, NULL, NULL},
    {"c64t Histogram16",                DVP_KN_VLIB_HISTOGRAM_16,           0, NULL, NULL},
    {"c64t WeightedHistogram16",        DVP_KN_VLIB_WEIGHTED_HISTOGRAM_16,  0, NULL, NULL},
    {"c64t HistogramnD16",              DVP_KN_VLIB_HISTOGRAM_ND_16,        0, NULL, NULL},
    {"c64t BhattacharyyaDistance",      DVP_KN_VLIB_BHATTACHAYA_DISTANCE,   0, NULL, NULL},
    {"c64t L1Distance",                 DVP_KN_VLIB_L1DISTANCE,             0, NULL, NULL},

    {"c64t xYxY to LUMA",               DVP_KN_XYXY_TO_Y800,                0, NULL, NULL},
    {"c64t UYVY to YUV420p",            DVP_KN_UYVY_TO_YUV420p,             0, NULL, NULL},
    {"c64t UYVY to YUV422p",            DVP_KN_UYVY_TO_YUV422p,             0, NULL, NULL},
    {"c64t UYVY to RGBp",               DVP_KN_UYVY_TO_RGBp,                0, NULL, NULL},
    {"c64t UYVY to HSLp",               DVP_KN_UYVY_TO_HSLp,                0, NULL, NULL},
    {"c64t UYVY to LABp",               DVP_KN_UYVY_TO_LABp,                0, NULL, NULL},
    {"c64t YUV422p to UYVY",            DVP_KN_YUV422p_TO_UYVY,             0, NULL, NULL},
    {"c64t NV12 to UYVY444p",           DVP_KN_NV12_TO_YUV444p,             0, NULL, NULL},

    {"c64t xYxY to LUMA",               DVP_KN_VLIB_XYXY_TO_Y800,           0, NULL, NULL},
    {"c64t UYVY to YUV420p",            DVP_KN_VLIB_UYVY_TO_YUV420p,        0, NULL, NULL},
    {"c64t UYVY to YUV422p",            DVP_KN_VLIB_UYVY_TO_YUV422p,        0, NULL, NULL},
    {"c64t UYVY to RGBp",               DVP_KN_VLIB_UYVY_TO_RGBp,           0, NULL, NULL},
    {"c64t UYVY to HSLp",               DVP_KN_VLIB_UYVY_TO_HSLp,           0, NULL, NULL},
    {"c64t UYVY to LABp",               DVP_KN_VLIB_UYVY_TO_LABp,           0, NULL, NULL},
    {"c64t YUV422p to UYVY",            DVP_KN_VLIB_YUV422p_TO_UYVY,        0, NULL, NULL},

    {"c64t Disparity8",                 DVP_KN_VLIB_DISPARITY_SAD8,         0, NULL, NULL},
    {"c64t Disparity16",                DVP_KN_VLIB_DISPARITY_SAD16,        0, NULL, NULL},
    {"c64t MeanLuma16",                 DVP_KN_VLIB_INIT_MEAN_LUMA_S16,     0, NULL, NULL},
    {"c64t MeanLuma32",                 DVP_KN_VLIB_INIT_MEAN_LUMA_S32,     0, NULL, NULL},
    {"c64t VarConst16",                 DVP_KN_VLIB_INIT_VAR_CONST_S16,     0, NULL, NULL},
    {"c64t VarConst32",                 DVP_KN_VLIB_INIT_VAR_CONST_S32,     0, NULL, NULL},
    {"c64t Legendre Moments Init",      DVP_KN_VLIB_LEGENDRE_MOMENTS_INIT,  0, NULL, NULL},
    {"c64t HarrisScore7x7 32bit",       DVP_KN_VLIB_HARRIS_SCORE_7x7_U32,   0, NULL, NULL},
    {"c64t Nonmaxsupress 32bit",        DVP_KN_VLIB_NONMAXSUPPRESS_U32,     0, &dsp_nonmax_shift3, NULL},
#endif
#ifdef DVP_USE_IMGLIB
    {"c64t YUV420p to RGB565",          DVP_KN_YUV422p_TO_RGB565,           0, NULL, NULL},
    {"c64t YUV420p to RGB565",          DVP_KN_IMG_YUV422p_TO_RGB565,       0, NULL, NULL},

    {"c64t Img Conv 3x3",               DVP_KN_CONV_3x3,                    0, &dsp_shift3, NULL},
    {"c64t Img Conv 5x5",               DVP_KN_CONV_5x5,                    0, &dsp_shift5, NULL},
    {"c64t Img Conv 7x7",               DVP_KN_CONV_7x7,                    0, &dsp_shift7, NULL},

    {"c64t Img Conv 3x3",               DVP_KN_IMG_CONV_3x3,                0, &dsp_shift3, NULL},
    {"c64t Img Conv 5x5",               DVP_KN_IMG_CONV_5x5,                0, &dsp_shift5, NULL},
    {"c64t Img Conv 7x7",               DVP_KN_IMG_CONV_7x7,                0, &dsp_shift7, NULL},
    {"c64t Img Conv 11x11",             DVP_KN_IMG_CONV_11x11,              0, &dsp_shift11, NULL},
    {"c64t Img Conv 5x5 i8 c16s",       DVP_KN_IMG_CONV_5x5_I8_C16,         0, &dsp_shift5, NULL},
    {"c64t Img Conv 7x7 i8 c16s",       DVP_KN_IMG_CONV_7x7_I8_C16,         0, &dsp_shift7, NULL},
    {"c64t Img Conv 3x3 i16s c16s",     DVP_KN_IMG_CONV_3x3_I16s_C16,       0, &dsp_shift3, NULL},
    {"c64t Img Conv 5x5 i16 c16s",      DVP_KN_IMG_CONV_5x5_I16s_C16,       0, &dsp_shift5, NULL},
    {"c64t Img Conv 7x7 i16 c16s",      DVP_KN_IMG_CONV_7x7_I16s_C16,       0, &dsp_shift7, NULL},
    {"c64t Img Conv 11x11 i16s c16s",   DVP_KN_IMG_CONV_11x11_I16s_C16,     0, &dsp_shift11, NULL},

    {"c64t Img Thr gt2max8",            DVP_KN_THR_GT2MAX_8,                0, NULL, NULL},
    {"c64t Img Thr gt2max16",           DVP_KN_THR_GT2MAX_16,               0, NULL, NULL},
    {"c64t Img Thr gt2thr8",            DVP_KN_THR_GT2THR_8,                0, NULL, NULL},
    {"c64t Img Thr gt2thr16",           DVP_KN_THR_GT2THR_16,               0, NULL, NULL},
    {"c64t Img Thr le2min8",            DVP_KN_THR_LE2MIN_8,                0, NULL, NULL},
    {"c64t Img Thr le2min16",           DVP_KN_THR_LE2MIN_16,               0, NULL, NULL},
    {"c64t Img Thr le2thr8",            DVP_KN_THR_LE2THR_8,                0, NULL, NULL},
    {"c64t Img Thr le2thr16",           DVP_KN_THR_LE2THR_16,               0, NULL, NULL},
    {"c64t Img sobel 3x3_8",            DVP_KN_SOBEL_3x3_8,                 0, &dsp_shift3, NULL},

    {"c64t Img Thr gt2max8",            DVP_KN_IMG_THR_GT2MAX_8,            0, NULL, NULL},
    {"c64t Img Thr gt2max16",           DVP_KN_IMG_THR_GT2MAX_16,           0, NULL, NULL},
    {"c64t Img Thr gt2thr8",            DVP_KN_IMG_THR_GT2THR_8,            0, NULL, NULL},
    {"c64t Img Thr gt2thr16",           DVP_KN_IMG_THR_GT2THR_16,           0, NULL, NULL},
    {"c64t Img Thr le2min8",            DVP_KN_IMG_THR_LE2MIN_8,            0, NULL, NULL},
    {"c64t Img Thr le2min16",           DVP_KN_IMG_THR_LE2MIN_16,           0, NULL, NULL},
    {"c64t Img Thr le2thr8",            DVP_KN_IMG_THR_LE2THR_8,            0, NULL, NULL},
    {"c64t Img Thr le2thr16",           DVP_KN_IMG_THR_LE2THR_16,           0, NULL, NULL},
    {"c64t Img sobel 3x3_8",            DVP_KN_IMG_SOBEL_3x3_8,             0, &dsp_shift3, NULL},
    {"c64t Img sobel 3x3_16s",          DVP_KN_IMG_SOBEL_3x3_16s,           0, &dsp_shift3, NULL},
    {"c64t Img sobel 5x5_16s",          DVP_KN_IMG_SOBEL_5x5_16s,           0, &dsp_shift5, NULL},
    {"c64t Img sobel 5x5_16s",          DVP_KN_IMG_SOBEL_7x7_16s,           0, &dsp_shift7, NULL},
    {"c64t CannyImgSmooth",             DVP_KN_CANNY_IMAGE_SMOOTHING,       0, &dsp_shift7, NULL},
    {"c64t Img Clipping 16",            DVP_KN_IMG_CLIPPING_16,             0, NULL, NULL},
    {"c64t Img Boundary8",              DVP_KN_IMG_BOUNDARY_8,              0, NULL, NULL},
    {"c64t Img Boundary16s",            DVP_KN_IMG_BOUNDARY_16s,            0, NULL, NULL},
    {"c64t Img Correlation 3x3",        DVP_KN_IMG_CORR_3x3,                0, &dsp_shift3, NULL},
    {"c64t Img Correlation 3x3 I8 C16s",  DVP_KN_IMG_CORR_3x3_I8_C16s,      0, &dsp_shift3, NULL},
    {"c64t Img Correlation 3x3 I16s C16s",  DVP_KN_IMG_CORR_3x3_I16s_C16s,  0, &dsp_shift3, NULL},
    {"c64t Img Correlation 5x5 I16s C16s",  DVP_KN_IMG_CORR_5x5_I16s_C16s,  0, &dsp_shift5, NULL},
    {"c64t Img Correlation 11x11 I8 C16s",  DVP_KN_IMG_CORR_11x11_I8_C16s,  0, &dsp_shift11, NULL},
    {"c64t Img Correlation 11x11 I16s C16s",  DVP_KN_IMG_CORR_11x11_I16s_C16s, 0, &dsp_shift11, NULL},
    {"c64t Img Correlation Gen I16 C16s",  DVP_KN_IMG_CORR_GEN_I16s_C16s,   0, &dsp_shift11, NULL},
    {"c64t Img Correlation Gen Iq",     DVP_KN_IMG_CORR_GEN_IQ,             0, NULL, NULL},
    {"c64t Img Histogram8",             DVP_KN_IMG_HISTOGRAM_8,             0, NULL, NULL},
    {"c64t Img Histogram16",            DVP_KN_IMG_HISTOGRAM_16,            0, NULL, NULL},
    {"c64t Img Median 3x3 8",           DVP_KN_IMG_MEDIAN_3x3_8,            0, &dsp_shift3, NULL},
    {"c64t Img Median 3x3 16s",          DVP_KN_IMG_MEDIAN_3x3_16s,         0, &dsp_shift3, NULL},
    {"c64t Img Perimeter 8",            DVP_KN_IMG_PERIMETER_8,             0, &dsp_shift3, NULL},
    {"c64t Img Perimeter 16",           DVP_KN_IMG_PERIMETER_16,            0, &dsp_shift3, NULL},
    {"c64t Img Demux LE 8",             DVP_KN_IMG_YC_DEMUX_LE16_8,         0, NULL, NULL},
    {"c64t Img Demux BE 8",             DVP_KN_IMG_YC_DEMUX_BE16_8,         0, NULL, NULL},
    {"c64t Img Pix Sat ",               DVP_KN_IMG_PIX_SAT,                 0, NULL, NULL},
    {"c64t Img Pix Expand ",            DVP_KN_IMG_PIX_EXPAND,              0, NULL, NULL},
    {"c64t Img SAD 3x3",                DVP_KN_IMG_SAD_3x3,                 0, &dsp_shift3, NULL},
    {"c64t Img SAD 5x5",                DVP_KN_IMG_SAD_5x5,                 0, &dsp_shift5, NULL},
    {"c64t Img SAD 7x7",                DVP_KN_IMG_SAD_7x7,                 0, &dsp_shift7, NULL},
    {"c64t Img SAD 8x8",                DVP_KN_IMG_SAD_8x8,                 0, &dsp_shift8, NULL},
    {"c64t Img SAD 16x16",              DVP_KN_IMG_SAD_16x16,               0, &dsp_shift16, NULL},
    {"c64t Img ErrDiff 8",              DVP_KN_IMG_ERRDIFF_BIN_8,           0, NULL, NULL},
    {"c64t Img ErrDiff 16",             DVP_KN_IMG_ERRDIFF_BIN_16,          0, NULL, NULL},
    {"c64t Img FDCT 8x8",               DVP_KN_IMG_FDCT_8x8,                0, NULL, NULL},
    {"c64t Img IFDCT 8x8",              DVP_KN_IMG_IDCT_8x8_12Q4,           0, NULL, NULL},
    {"c64t Img MAD 8x8",                DVP_KN_IMG_MAD_8x8,                 0, NULL, NULL},
    {"c64t Img MAD 16x16",              DVP_KN_IMG_MAD_16x16,               0, NULL, NULL},
    {"c64t Img Quantization 16",        DVP_KN_IMG_QUANTIZE_16,             0, NULL, NULL},
    {"c64t Img Wavelet Horz",           DVP_KN_IMG_WAVE_HORZ,               0, NULL, NULL},

#endif
#ifdef DVP_USE_DSPLIB
    {"c64t DSP Add 16 ",                DVP_KN_DSP_ADD16,                   0, NULL, NULL},
    {"c64t DSP Add 32 ",                DVP_KN_DSP_ADD32,                   0, NULL, NULL},
    {"c64t DSP Autocorrelation ",       DVP_KN_DSP_AUTOCORR16,              0, NULL, NULL},
    {"c64t DSP Bit exp ",               DVP_KN_DSP_BITEXP32,                0, NULL, NULL},
    {"c64t DSP Block swap 16 ",         DVP_KN_DSP_BLKESWAP16,              0, NULL, NULL},
    {"c64t DSP Block swap 32 ",         DVP_KN_DSP_BLKESWAP32,              0, NULL, NULL},
    {"c64t DSP Block swap 64 ",         DVP_KN_DSP_BLKESWAP64,              0, NULL, NULL},
    {"c64t DSP Block move ",            DVP_KN_DSP_BLKMOVE,                 0, NULL, NULL},
    {"c64t DSP Dot product square ",    DVP_KN_DSP_DOTPRODSQR,              0, NULL, NULL},
    {"c64t DSP Dot product ",           DVP_KN_DSP_DOTPROD,                 0, NULL, NULL},
    {"c64t DSP FFT 16x16 ",             DVP_KN_DSP_FFT_16x16,               0, NULL, NULL},
    {"c64t DSP FFT 16x16R ",            DVP_KN_DSP_FFT_16x16R,              0, NULL, NULL},
    {"c64t DSP FFT 16x16IMRE ",         DVP_KN_DSP_FFT_16x16_IMRE,          0, NULL, NULL},
    {"c64t DSP FFT 16x32 ",             DVP_KN_DSP_FFT_16x32,               0, NULL, NULL},
    {"c64t DSP FFT 32x32 ",             DVP_KN_DSP_FFT_32x32,               0, NULL, NULL},
    {"c64t DSP FFT 32x32s ",            DVP_KN_DSP_FFT_32x32s,              0, NULL, NULL},

    {"c64t DSP FIR Complex ",           DVP_KN_DSP_FIR_CPLX,                0, NULL, NULL},
    {"c64t DSP FIR HM Radix 4 ",        DVP_KN_DSP_FIR_HM4x4,               0, NULL, NULL},
    {"c64t DSP FIR General",            DVP_KN_DSP_FIR_GEN,                 0, NULL, NULL},
    {"c64t DSP FIR HM17 Radix 8 ",      DVP_KN_DSP_FIR_HM17_RA8x8,          0, NULL, NULL},
    {"c64t DSP FIR Adaptive LMS ",      DVP_KN_DSP_FIR_LMS2,                0, NULL, NULL},
    {"c64t DSP FIR Radix 4 ",           DVP_KN_DSP_FIR_R4,                  0, NULL, NULL},
    {"c64t DSP FIR Radix  8",           DVP_KN_DSP_FIR_R8,                  0, NULL, NULL},
    {"c64t DSP FIR HM16 Radix 8  ",     DVP_KN_DSP_FIR_HM16_RM8A8x8,        0, NULL, NULL},
    {"c64t DSP FIR Symmetric ",         DVP_KN_DSP_FIR_SYM,                 0, NULL, NULL},

    {"c64t DSP IFFT 16x16 ",            DVP_KN_DSP_IFFT_16x16,              0, NULL, NULL},
    {"c64t DSP IFFT 16x16 complex",     DVP_KN_DSP_IFFT_16x16_IMRE,         0, NULL, NULL},
    {"c64t DSP IFFT 16x32 ",            DVP_KN_DSP_IFFT_16x32,              0, NULL, NULL},
    {"c64t DSP IFFT 32x32 ",            DVP_KN_DSP_IFFT_32x32,              0, NULL, NULL},

    {"c64t DSP IIR ",                   DVP_KN_DSP_IIR,                     0, NULL, NULL},
    {"c64t DSP IIR Lattice",            DVP_KN_DSP_IIR_LAT,                 0, NULL, NULL},
    {"c64t DSP IIR Single Point ",      DVP_KN_DSP_IIR_SS,                  0, NULL, NULL},

    {"c64t DSP Lib MatrixMul ",         DVP_KN_DSP_MUL,                     0, NULL, NULL},
    {"c64t DSP Lib MatrixMul Complex",  DVP_KN_DSP_MUL_CPLX, 0, NULL, NULL},
    {"c64t DSP Lib MatrixTranspose ",   DVP_KN_DSP_MAT_TRANS,               0, NULL, NULL},
    {"c64t DSP Lib MaxIdx ",            DVP_KN_DSP_MAXIDX,                  0, NULL, NULL},
    {"c64t DSP Lib MaxVal ",            DVP_KN_DSP_MAXVAL,                  0, NULL, NULL},
    {"c64t DSP Lib MinVal ",            DVP_KN_DSP_MINVAL,                  0, NULL, NULL},
    {"c64t DSP Lib MInError ",          DVP_KN_DSP_MINERROR,                0, NULL, NULL},
    {"c64t DSP Lib Mult32 ",            DVP_KN_DSP_MUL32,                   0, NULL, NULL},
    {"c64t DSP Lib Negate32 ",          DVP_KN_DSP_NEG32,                   0, NULL, NULL},
    {"c64t DSP Lib Reciprocal16 ",      DVP_KN_DSP_RECIP16,                 0, NULL, NULL},
    {"c64t DSP Lib SumofSquares ",      DVP_KN_DSP_VECSUMSQ,                0, NULL, NULL},
    {"c64t DSP Lib WeightedVector ",    DVP_KN_DSP_W_VEC,                   0, NULL, NULL},

    {"c64t DSP Lib FFT Twiddle16x16 ",  DVP_KN_GEN_TWIDDLE_FFT_16X16,       0, NULL, NULL},
    {"c64t DSP Lib FFT Twiddle16x16 ",  DVP_KN_GEN_TWIDDLE_FFT_16X16_IMRE,  0, NULL, NULL},
    {"c64t DSP Lib FFT Twiddle16x32 ",  DVP_KN_GEN_TWIDDLE_FFT_16X32,       0, NULL, NULL},
    {"c64t DSP Lib FFT Twiddle 32x32 ", DVP_KN_GEN_TWIDDLE_FFT_32x32,       0, NULL, NULL},
    {"c64t DSP Lib IFFT Twiddle16x16 ", DVP_KN_GEN_TWIDDLE_IFFT_16X16,      0, NULL, NULL},
    {"c64t DSP Lib IFFT Twiddle16x16 ", DVP_KN_GEN_TWIDDLE_IFFT_16X16_IMRE, 0, NULL, NULL},
#endif
#ifdef DVP_USE_TISMO
    {"c64t  TISMO",                     DVP_KN_TISMO_DISPARITY,             0, NULL, NULL},
#endif
#ifdef DVP_USE_TISMOV02
    {"c64t  TISMOV02",                  DVP_KN_TISMOV02_DISPARITY,          0, NULL, NULL},
#endif
#ifdef DVP_USE_RVM
    {"c64t RVM",                        DVP_KN_RVM,                         0, NULL, NULL},
#endif
#ifdef DVP_USE_ORB
    {"c64t  KN_ORB",                    DVP_KN_ORB,                         0, NULL, NULL},
#endif
#ifdef DVP_USE_DEI
    {"c64t Deinterlacer",               DVP_KN_DEI_DEINTERLACER,            0, NULL, NULL},
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
#ifdef DVP_USE_VLIB
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_VLIB enabled!\n");
#endif
#ifdef DVP_USE_RVM
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_RVM enabled!\n");
#endif
#ifdef DVP_USE_TISMO
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_TISMO enabled!\n");
#endif
#ifdef DVP_USE_TISMOV02
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" DVP_USE_TISMOV02 enabled!\n");
#endif
    DVP_PRINT(DVP_ZONE_KGM, KGM_TAG" supports %u kernels\n", numRemoteKernels);
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
    return DVP_CORE_DSP;
}

MODULE_EXPORT DVP_U32 DVP_GetMaximumLoad(void)
{
#if defined(TARGET_DVP_OMAP4)
    return 466; // 466 Mhz
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
static fourcc_t colorY16[]  = {FOURCC_Y16};
static fourcc_t colorY32[]  = {FOURCC_Y32};
static fourcc_t colorUYVY[] = {FOURCC_UYVY};

static DVP_U32 DVP_KernelGraphManager_DSP(DVP_KernelNode_t *pNodes, DVP_U32 startNode, DVP_U32 numNodes)
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
            DVP_PRINT(DVP_ZONE_ERROR, KGM_TAG": ERROR! Kernel Graph Manager for "KGM_TAG" is uninitialized!\n");
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
#ifdef DVP_USE_VLIB
                case DVP_KN_XYXY_TO_Y800: // single planes
                case DVP_KN_UYVY_TO_RGBp: // multiplane output
                case DVP_KN_UYVY_TO_YUV420p:
                case DVP_KN_UYVY_TO_YUV422p:
                case DVP_KN_YUV422p_TO_UYVY:
                case DVP_KN_NV12_TO_YUV444p:
                case DVP_KN_INTEGRAL_IMAGE_8:
                case DVP_KN_VLIB_XYXY_TO_Y800:
                case DVP_KN_VLIB_UYVY_TO_RGBp:
                case DVP_KN_VLIB_UYVY_TO_YUV420p:
                case DVP_KN_VLIB_UYVY_TO_YUV422p:
                case DVP_KN_VLIB_YUV422p_TO_UYVY:
                case DVP_KN_VLIB_INTEGRAL_IMAGE_8:
                case DVP_KN_VLIB_INTEGRAL_IMAGE_16:
                case DVP_KN_VLIB_PACK_MASK_32:
                case DVP_KN_VLIB_UNPACK_MASK_32:
                case DVP_KN_VLIB_IMAGE_PYRAMID_8:
                case DVP_KN_VLIB_IMAGE_PYRAMID_16:
                case DVP_KN_VLIB_INIT_MEAN_LUMA_S16:
                case DVP_KN_VLIB_INIT_MEAN_LUMA_S32:
                case DVP_KN_VLIB_EXTRACT_BACK_8_16:
                {
                    DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIO->input);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIO->output);
                    break;
                }
                case DVP_KN_DILATE_CROSS:
                case DVP_KN_DILATE_SQUARE:
                case DVP_KN_DILATE_MASK:
                case DVP_KN_ERODE_CROSS:
                case DVP_KN_ERODE_MASK:
                case DVP_KN_ERODE_SQUARE:
                case DVP_KN_VLIB_DILATE_CROSS:
                case DVP_KN_VLIB_DILATE_SQUARE:
                case DVP_KN_VLIB_DILATE_MASK:
                case DVP_KN_VLIB_ERODE_CROSS:
                case DVP_KN_VLIB_ERODE_MASK:
                case DVP_KN_VLIB_ERODE_SQUARE:
                case DVP_KN_VLIB_ERODE_SINGLEPIXEL:
                {
                    DVP_Morphology_t *pMorph = dvp_knode_to(&pNodes[n], DVP_Morphology_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);

                    DVP_PrintImage(DVP_ZONE_KGM, &pMorph->input);
                    DVP_PrintImage(DVP_ZONE_KGM, &pMorph->output);

                    if (pNodes[n].header.kernel == DVP_KN_ERODE_MASK ||
                        pNodes[n].header.kernel == DVP_KN_DILATE_MASK)
                    {
                        dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->mask, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                        DVP_PrintImage(DVP_ZONE_KGM, &pMorph->mask);
                    }
                    break;
                }
                case DVP_KN_IIR_HORZ:
                case DVP_KN_IIR_VERT:
                case DVP_KN_VLIB_IIR_HORZ:
                case DVP_KN_VLIB_IIR_VERT:
                case DVP_KN_VLIB_IIR_HORZ_16:
                case DVP_KN_VLIB_IIR_VERT_16:
                {
                    DVP_IIR_t *pIIR = dvp_knode_to(&pNodes[n], DVP_IIR_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIIR->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIIR->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIIR->scratch, DVP_FALSE, (DVP_PTR)pTmp, &translations);
#if !defined(__QNX__) /// @todo REMOVE: Temporary to avoid build error in QNX
                    if (pIIR->bounds[0].pData)
                    {
#if defined(DVP_USE_RCM)
                        DVP_COMPLAIN_IF_FALSE(dvp_rpc_flush(rpc, DVP_GetSupportedRemoteCore(), (DVP_PTR)pIIR->bounds[0].pData, pIIR->bounds[0].height*pIIR->bounds[0].width, DVP_MTYPE_MPUCACHED_VIRTUAL));
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
                case DVP_KN_VLIB_CCL:
                {
                    DVP_CCL_t *pCCL = dvp_knode_to(&pNodes[n], DVP_CCL_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCCL->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCCL->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pCCL->input);
                    DVP_PrintImage(DVP_ZONE_KGM, &pCCL->output);
                    break;
                }
                case DVP_KN_CANNY_2D_GRADIENT:
                case DVP_KN_VLIB_CANNY_2D_GRADIENT:
                {
                    DVP_Canny2dGradient_t *pG = dvp_knode_to(&pNodes[n], DVP_Canny2dGradient_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pG->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pG->outGradX, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pG->outGradY, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pG->outMag, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_CANNY_NONMAX_SUPPRESSION:
                case DVP_KN_VLIB_CANNY_NONMAX_SUPPRESSION:
                {
                    DVP_CannyNonMaxSuppression_t *pCnonmax = dvp_knode_to(&pNodes[n], DVP_CannyNonMaxSuppression_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inMag, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inGradX, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inGradY, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_GRADIENT_V5x5_PYRAMID_8:
                case DVP_KN_VLIB_GRADIENT_H5x5_PYRAMID_8:
                case DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_8:
                case DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_16:
                {
                    DVP_Gradient_t *pGrad = dvp_knode_to(&pNodes[n], DVP_Gradient_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->scratch, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_CANNY_HYST_THRESHHOLD:
                case DVP_KN_VLIB_CANNY_HYST_THRESHHOLD:
                case DVP_KN_VLIB_HYST_THRESHOLD:
                {
                    DVP_CannyHystThresholding_t *pDth = dvp_knode_to(&pNodes[n], DVP_CannyHystThresholding_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inMag, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inEdgeMap, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_NONMAXSUPPRESS_3x3_S16:
                case DVP_KN_NONMAXSUPPRESS_5x5_S16:
                case DVP_KN_NONMAXSUPPRESS_7x7_S16:
                case DVP_KN_VLIB_NONMAXSUPPRESS_3x3_S16:
                case DVP_KN_VLIB_NONMAXSUPPRESS_5x5_S16:
                case DVP_KN_VLIB_NONMAXSUPPRESS_7x7_S16:
                {
                    DVP_Threshold_t *pNonmax = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pNonmax->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pNonmax->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pNonmax->input);
                    DVP_PrintImage(DVP_ZONE_KGM, &pNonmax->output);
                    break;
                }
                case DVP_KN_VLIB_INIT_HISTOGRAM_8:
                case DVP_KN_VLIB_INIT_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->edges, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_HISTOGRAM_8:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->edges, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[0], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[1], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[2], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pH->input);
                    break;
                }
                case DVP_KN_VLIB_WEIGHTED_HISTOGRAM_8:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->edges, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->binWeights, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[0], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[1], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[2], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pH->input);
                    break;
                }
                case DVP_KN_VLIB_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->edges, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[0], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pH->input);
                    break;
                }
                case DVP_KN_VLIB_WEIGHTED_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->edges, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->binWeights, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[0], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pH->input);
                    break;
                }
                case DVP_KN_VLIB_HISTOGRAM_ND_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->numBinsArray, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[0], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[1], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[2], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pH->input);
                    break;
                }
                case DVP_KN_VLIB_BHATTACHAYA_DISTANCE:
                case DVP_KN_VLIB_L1DISTANCE:
                {
                    DVP_Distance_t *pDist = dvp_knode_to(&pNodes[n], DVP_Distance_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDist->X, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDist->Y, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_INIT_VAR_CONST_S16:
                case DVP_KN_VLIB_INIT_VAR_CONST_S32:
                {
                    DVP_ConstVar_t *pV = dvp_knode_to(&pNodes[n], DVP_ConstVar_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pV->var, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_DISPARITY_SAD8:
                case DVP_KN_VLIB_DISPARITY_SAD16:
                {
                    DVP_Disparity_t *pT = dvp_knode_to(&pNodes[n], DVP_Disparity_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->left, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->right, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->cost, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->mincost, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->disparity, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_SUB_BACK_S16:
                case DVP_KN_VLIB_SUB_BACK_S32:
                {
                    DVP_SubBackground_t *pSB = dvp_knode_to(&pNodes[n], DVP_SubBackground_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pSB->newestData, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pSB->currentMean, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pSB->currentVar, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pSB->output32packed, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_MOG_S16:
                case DVP_KN_VLIB_MOG_S32:
                {
                    DVP_Mog_t *pMOG = dvp_knode_to(&pNodes[n], DVP_Mog_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->image, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->wts, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->means, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->vars, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->compIndex, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->scratch, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->fgmask, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_NORMALFLOW_16:
                {
                    DVP_NormalFlow_t *pNF = dvp_knode_to(&pNodes[n], DVP_NormalFlow_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->mag, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->x, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->y, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->lut, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->u, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->v, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_16:
                case DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_3D:
                {
                    DVP_Simplex_t *pS = dvp_knode_to(&pNodes[n], DVP_Simplex_t);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->start, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->step, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->stop, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[0], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[1], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[2], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[3], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[4], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[5], DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_HARRIS_SCORE_7x7:
                case DVP_KN_VLIB_HARRIS_SCORE_7x7_U32:
                {
                    DVP_HarrisDSP_t *pHs = dvp_knode_to(&pNodes[n], DVP_HarrisDSP_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pHs->inGradX, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pHs->inGradY, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHs->scratch, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pHs->harrisScore, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_TRACK_FEATURES_LUCAS_7x7:
                {
                    DVP_TrackFeatures_t *pLk = dvp_knode_to(&pNodes[n], DVP_TrackFeatures_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLk->im1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLk->im2, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLk->gradx, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLk->grady, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLk->x, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLk->y, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLk->outx, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLk->outy, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLk->scratch, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_HOUGH_LINE_FROM_LIST:
                {
                    DVP_HoughLine_t *pHl = dvp_knode_to(&pNodes[n], DVP_HoughLine_t);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->pEdgeMapList, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->pSIN, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->pCOS, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->ping, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->pong, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->pang, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->peng, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pHl->pOutHoughSpace, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_EWR_MEAN_S16:
                case DVP_KN_VLIB_EWR_MEAN_S32:
                {
                    DVP_EMean_t *pEWR = dvp_knode_to(&pNodes[n], DVP_EMean_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->mean, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->data, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->mask, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_EWR_VAR_S16:
                case DVP_KN_VLIB_EWR_VAR_S32:
                {
                    DVP_EMean_t *pEWR = dvp_knode_to(&pNodes[n], DVP_EMean_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->var, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->mean, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->data, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->mask, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_UWR_MEAN_S16:
                {
                    DVP_UMean_t *pUWR = dvp_knode_to(&pNodes[n], DVP_UMean_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_mean, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_data, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_data, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_mask, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_mask, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_mean, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_UWR_VAR_S16:
                {
                    DVP_UMean_t *pUWR = dvp_knode_to(&pNodes[n], DVP_UMean_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_var, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_mean, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_mean, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_data, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_data, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_mask, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_mask, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_var, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_LEGENDRE_MOMENTS_INIT:
                {
                    DVP_LegendreMoments_t *pLgm = dvp_knode_to(&pNodes[n], DVP_LegendreMoments_t);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pLgm->lPoly, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_LEGENDRE_MOMENTS:
                {
                    DVP_LegendreMoments_t *pLgm = dvp_knode_to(&pNodes[n], DVP_LegendreMoments_t);
                    dvp_rpc_prepare_image(rpc, DVP_CORE_DSP, &pLgm->im, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pLgm->lPoly, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pLgm->lMoments, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_UYVY_TO_HSLp: // multiplane output
                case DVP_KN_VLIB_UYVY_TO_HSLp: // multiplane output
                {
                    DVP_Int2Pl_t *pLAB = dvp_knode_to(&pNodes[n], DVP_Int2Pl_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output1, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output2, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output3, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_UYVY_TO_LABp:
                case DVP_KN_VLIB_UYVY_TO_LABp:
                {
                    DVP_Int2Pl_t *pLAB = dvp_knode_to(&pNodes[n], DVP_Int2Pl_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output1, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output2, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output3, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLAB->scratch, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_NONMAXSUPPRESS_U32:
                {
                    DVP_Nonmax_NxN_t *pNonmax32 = dvp_knode_to(&pNodes[n], DVP_Nonmax_NxN_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pNonmax32->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pNonmax32->scratch, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pNonmax32->pixIndex, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_VLIB_KALMAN_2x4:
                case DVP_KN_VLIB_KALMAN_4x6:
                {
                    DVP_KalmanFilter_t *pKf = dvp_knode_to(&pNodes[n], DVP_KalmanFilter_t);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->inMeasurements, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->transition, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->errorCov, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->predictedErrorCov, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->state, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->predictedState, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->measurement, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->processNoiseCov, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->measurementNoiseCov, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->kalmanGain, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->temp1, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->temp2, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->temp3, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
#endif
#ifdef DVP_USE_TISMO
                case DVP_KN_TISMO_DISPARITY:
                {
                    DVP_Tismo_t *pT = dvp_knode_to(&pNodes[n], DVP_Tismo_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->left, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->right, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->invalid, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->left);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->right);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->output);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->invalid);
                    break;
                }
#endif
#ifdef DVP_USE_TISMOV02
                case DVP_KN_TISMOV02_DISPARITY:
                {
                    DVP_TismoV02_t *pT = dvp_knode_to(&pNodes[n], DVP_TismoV02_t);

                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->in_left, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->in_right, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->in_leftIntegralImage, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->in_rightIntegralImage, DVP_TRUE, (DVP_PTR)pTmp, &translations);

                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->out_raw, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->out_invalid, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->out_confidence, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->out_matchScore, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pT->out_lumaU8, DVP_FALSE, (DVP_PTR)pTmp, &translations);

                    DVP_PrintImage(DVP_ZONE_KGM, &pT->in_left);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->in_right);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->in_leftIntegralImage);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->in_rightIntegralImage);

                    DVP_PrintImage(DVP_ZONE_KGM, &pT->out_raw);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->out_invalid);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->out_confidence);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->out_matchScore);
                    DVP_PrintImage(DVP_ZONE_KGM, &pT->out_lumaU8);
                    break;
                }
#endif
#ifdef DVP_USE_ORB
                case DVP_KN_ORB:
                {
                  DVP_Orb_t * pOrb = dvp_knode_to(&pNodes[n], DVP_Orb_t);
                  dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pOrb->inputImage, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                  dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pOrb->filtImage, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                  dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pOrb->harrisImage, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                  dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pOrb->integralImage, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                  dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pOrb->orbPattern31, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                  dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pOrb->orbOutput.orbData, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                  dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pOrb->orbOutput.outImage, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                  break;
                }
#endif
#ifdef DVP_USE_DEI
                case DVP_KN_DEI_DEINTERLACER:
                {
                    DVP_Deinterlacer_t *pD = dvp_knode_to(&pNodes[n], DVP_Deinterlacer_t);

                    if (pD->phy_virt_flag == 0) {
                        dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pD->phy_fld_in_current, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                        dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pD->phy_fld_in_prev,    DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    }

                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pD->phy_luma_d0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pD->phy_luma_d1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pD->phy_luma_d2, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
#endif
#ifdef DVP_USE_RVM
                case DVP_KN_RVM:
                {
                    DVP_Rvm_t * pRvm = dvp_knode_to(&pNodes[n], DVP_Rvm_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pRvm->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pRvm->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pRvm->lut, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
#endif
#ifdef DVP_USE_IMGLIB
                case DVP_KN_IMG_BOUNDARY_8:
                case DVP_KN_IMG_BOUNDARY_16s:
                {
                    DVP_Boundary_t *pBoundary = dvp_knode_to(&pNodes[n], DVP_Boundary_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pBoundary->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pBoundary->outCoord, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pBoundary->outGray, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pBoundary->input);
                    break;
                }
                case DVP_KN_YUV422p_TO_RGB565:
                case DVP_KN_SOBEL_3x3_8:
                case DVP_KN_IMG_YUV422p_TO_RGB565:
                case DVP_KN_IMG_SOBEL_3x3_8:
                case DVP_KN_IMG_SOBEL_3x3_16s:
                case DVP_KN_IMG_SOBEL_5x5_16s:
                case DVP_KN_IMG_SOBEL_7x7_16s:
                case DVP_KN_IMG_PIX_SAT:
                case DVP_KN_IMG_PIX_EXPAND:
                case DVP_KN_IMG_PERIMETER_8:
                case DVP_KN_IMG_PERIMETER_16:
                case DVP_KN_IMG_MEDIAN_3x3_8:
                case DVP_KN_IMG_MEDIAN_3x3_16s:
                case DVP_KN_IMG_FDCT_8x8:
                case DVP_KN_IMG_IDCT_8x8_12Q4:
                {
                    DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIO->input);
                    DVP_PrintImage(DVP_ZONE_KGM, &pIO->output);
                    break;
                }
                case DVP_KN_IMG_CLIPPING_16:
                {
                    DVP_CannyHystThresholding_t *pDth = dvp_knode_to(&pNodes[n], DVP_CannyHystThresholding_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inMag, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inEdgeMap, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_IMG_HISTOGRAM_8:
                case DVP_KN_IMG_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    DVP_PrintImage(DVP_ZONE_KGM, &pH->input);
                    break;
                }
                case DVP_KN_CANNY_IMAGE_SMOOTHING:
                case DVP_KN_CONV_3x3:
                case DVP_KN_CONV_5x5:
                case DVP_KN_CONV_7x7:
                case DVP_KN_IMG_CONV_3x3:
                case DVP_KN_IMG_CONV_5x5:
                case DVP_KN_IMG_CONV_7x7:
                case DVP_KN_IMG_CONV_11x11:
                case DVP_KN_IMG_CONV_3x3_I16s_C16:
                case DVP_KN_IMG_CONV_7x7_I16s_C16:
                case DVP_KN_IMG_CONV_11x11_I16s_C16:
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->mask, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_IMG_SAD_3x3:
                case DVP_KN_IMG_SAD_5x5:
                case DVP_KN_IMG_SAD_7x7:
                case DVP_KN_IMG_SAD_8x8:
                case DVP_KN_IMG_SAD_16x16:
                case DVP_KN_IMG_MAD_8x8:
                case DVP_KN_IMG_MAD_16x16:
                {
                    DVP_SAD_t *pImg = dvp_knode_to(&pNodes[n], DVP_SAD_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->refImg, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_IMG_CONV_5x5_I8_C16:
                case DVP_KN_IMG_CONV_5x5_I16s_C16:
                case DVP_KN_IMG_CONV_7x7_I8_C16:
                {
                    DVP_ImageConvolution_with_buffer_t *pImg_buff = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_with_buffer_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg_buff->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg_buff->mask, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pImg_buff->scratch, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg_buff->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_THR_GT2MAX_8:
                case DVP_KN_THR_GT2MAX_16:
                case DVP_KN_THR_GT2THR_8:
                case DVP_KN_THR_GT2THR_16:
                case DVP_KN_THR_LE2MIN_8:
                case DVP_KN_THR_LE2MIN_16:
                case DVP_KN_THR_LE2THR_8:
                case DVP_KN_THR_LE2THR_16:
                case DVP_KN_IMG_THR_GT2MAX_8:
                case DVP_KN_IMG_THR_GT2MAX_16:
                case DVP_KN_IMG_THR_GT2THR_8:
                case DVP_KN_IMG_THR_GT2THR_16:
                case DVP_KN_IMG_THR_LE2MIN_8:
                case DVP_KN_IMG_THR_LE2MIN_16:
                case DVP_KN_IMG_THR_LE2THR_8:
                case DVP_KN_IMG_THR_LE2THR_16:
                case DVP_KN_IMG_ERRDIFF_BIN_8:
                case DVP_KN_IMG_ERRDIFF_BIN_16:
                {
                    DVP_Threshold_t *pImg = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_IMG_YC_DEMUX_BE16_8:
                case DVP_KN_IMG_YC_DEMUX_LE16_8:
                {
                    DVP_Int2Pl_t *pLAB = dvp_knode_to(&pNodes[n], DVP_Int2Pl_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output1, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output2, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output3, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_IMG_CORR_3x3:
                case DVP_KN_IMG_CORR_3x3_I8_C16s:
                case DVP_KN_IMG_CORR_3x3_I16s_C16s:
                case DVP_KN_IMG_CORR_5x5_I16s_C16s:
                case DVP_KN_IMG_CORR_11x11_I8_C16s:
                case DVP_KN_IMG_CORR_11x11_I16s_C16s:
                case DVP_KN_IMG_CORR_GEN_I16s_C16s:
                case DVP_KN_IMG_CORR_GEN_IQ:
                {
                    DVP_ImageCorrelation_t *pCOR = dvp_knode_to(&pNodes[n], DVP_ImageCorrelation_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCOR->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCOR->mask, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pCOR->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_IMG_QUANTIZE_16:
                {
                    DVP_ImageQuantization_t *pQnt = dvp_knode_to(&pNodes[n], DVP_ImageQuantization_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pQnt->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pQnt->recip_tbl, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_IMG_WAVE_HORZ:
                {
                    DVP_ImageWavelet_t *pWv = dvp_knode_to(&pNodes[n], DVP_ImageWavelet_t);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pWv->input, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pWv->qmf, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_GetSupportedRemoteCore(), &pWv->mqmf, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_image(rpc, DVP_GetSupportedRemoteCore(), &pWv->output, DVP_FALSE, (DVP_PTR)pTmp, &translations);
                    break;
                }
#endif
#ifdef DVP_USE_DSPLIB
        case DVP_KN_DSP_ADD16:
        case DVP_KN_DSP_ADD32:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }
                case DVP_KN_DSP_AUTOCORR16:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_BITEXP32:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_BLKESWAP16:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_BLKESWAP32:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_BLKESWAP64:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_BLKMOVE:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_DOTPRODSQR:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_DOTPROD:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_FFT_16x16:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_FFT_16x16R:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_FFT_16x16_IMRE:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_FFT_16x32:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_FFT_32x32:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }
                case DVP_KN_DSP_FFT_32x32s:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
                    break;
                }

        case DVP_KN_DSP_FIR_CPLX:
        case DVP_KN_DSP_FIR_HM4x4:
        case DVP_KN_DSP_FIR_GEN:
        case DVP_KN_DSP_FIR_HM17_RA8x8:
        case DVP_KN_DSP_FIR_R4:
        case DVP_KN_DSP_FIR_R8:
        case DVP_KN_DSP_FIR_HM16_RM8A8x8:
        case DVP_KN_DSP_FIR_SYM:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }
        case DVP_KN_DSP_FIR_LMS2:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_DSP_IFFT_16x16:
        case DVP_KN_DSP_IFFT_16x16_IMRE:
        case DVP_KN_DSP_IFFT_16x32:
        case DVP_KN_DSP_IFFT_32x32:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_DSP_IIR:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_DSP_IIR_LAT:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_DSP_IIR_SS:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_DSP_MUL:
        case DVP_KN_DSP_MUL_CPLX:
        {
            DVP_DSP_MatMul *pBuf = dvp_knode_to(&pNodes[n], DVP_DSP_MatMul);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }
        case DVP_KN_DSP_MAT_TRANS:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }
        case DVP_KN_DSP_MAXIDX:
        case DVP_KN_DSP_MAXVAL:
        case DVP_KN_DSP_MINVAL:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }
        case DVP_KN_DSP_MINERROR:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }
        case DVP_KN_DSP_MUL32:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }
        case DVP_KN_DSP_NEG32:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }
        case DVP_KN_DSP_RECIP16:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }
        case DVP_KN_DSP_VECSUMSQ:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }
        case DVP_KN_DSP_W_VEC:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_GEN_TWIDDLE_FFT_16X16:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_GEN_TWIDDLE_FFT_16X16_IMRE:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_GEN_TWIDDLE_FFT_16X32:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_GEN_TWIDDLE_FFT_32x32:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_GEN_TWIDDLE_IFFT_16X16:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
            break;
        }

        case DVP_KN_GEN_TWIDDLE_IFFT_16X16_IMRE:
        {
            DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_prepare_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_TRUE, (DVP_PTR)pTmp, &translations);
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
#ifdef DVP_USE_VLIB
                case DVP_KN_XYXY_TO_Y800: // single planes
                case DVP_KN_UYVY_TO_RGBp: // multiplane output
                case DVP_KN_UYVY_TO_YUV420p:
                case DVP_KN_UYVY_TO_YUV422p:
                case DVP_KN_YUV422p_TO_UYVY:
                case DVP_KN_NV12_TO_YUV444p:
                case DVP_KN_INTEGRAL_IMAGE_8:
                case DVP_KN_VLIB_XYXY_TO_Y800:
                case DVP_KN_VLIB_UYVY_TO_RGBp:
                case DVP_KN_VLIB_UYVY_TO_YUV420p:
                case DVP_KN_VLIB_UYVY_TO_YUV422p:
                case DVP_KN_VLIB_YUV422p_TO_UYVY:
                case DVP_KN_VLIB_INTEGRAL_IMAGE_8:
                case DVP_KN_VLIB_INTEGRAL_IMAGE_16:
                case DVP_KN_VLIB_PACK_MASK_32:
                case DVP_KN_VLIB_UNPACK_MASK_32:
                case DVP_KN_VLIB_IMAGE_PYRAMID_8:
                case DVP_KN_VLIB_IMAGE_PYRAMID_16:
                case DVP_KN_VLIB_INIT_MEAN_LUMA_S16:
                case DVP_KN_VLIB_INIT_MEAN_LUMA_S32:
                case DVP_KN_VLIB_EXTRACT_BACK_8_16:
                {
                    DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_DILATE_CROSS:
                case DVP_KN_DILATE_SQUARE:
                case DVP_KN_DILATE_MASK:
                case DVP_KN_ERODE_CROSS:
                case DVP_KN_ERODE_MASK:
                case DVP_KN_ERODE_SQUARE:
                case DVP_KN_VLIB_DILATE_CROSS:
                case DVP_KN_VLIB_DILATE_SQUARE:
                case DVP_KN_VLIB_DILATE_MASK:
                case DVP_KN_VLIB_ERODE_CROSS:
                case DVP_KN_VLIB_ERODE_MASK:
                case DVP_KN_VLIB_ERODE_SQUARE:
                case DVP_KN_VLIB_ERODE_SINGLEPIXEL:
                {
                    DVP_Morphology_t *pMorph = dvp_knode_to(&pNodes[n], DVP_Morphology_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->output, DVP_TRUE);
                    if (pNodes[n].header.kernel == DVP_KN_ERODE_MASK ||
                        pNodes[n].header.kernel == DVP_KN_DILATE_MASK)
                    {
                        dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMorph->mask, DVP_FALSE);
                    }
                    break;
                }
                case DVP_KN_IIR_HORZ:
                case DVP_KN_IIR_VERT:
                case DVP_KN_VLIB_IIR_HORZ:
                case DVP_KN_VLIB_IIR_VERT:
                case DVP_KN_VLIB_IIR_HORZ_16:
                case DVP_KN_VLIB_IIR_VERT_16:
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
                case DVP_KN_VLIB_CCL:
                {
                    DVP_CCL_t *pCCL = dvp_knode_to(&pNodes[n], DVP_CCL_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCCL->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCCL->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_CANNY_2D_GRADIENT:
                case DVP_KN_VLIB_CANNY_2D_GRADIENT:
                {
                    DVP_Canny2dGradient_t *pG = dvp_knode_to(&pNodes[n], DVP_Canny2dGradient_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pG->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pG->outGradX, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pG->outGradY, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pG->outMag, DVP_TRUE);
                    break;
                }
                case DVP_KN_CANNY_NONMAX_SUPPRESSION:
                case DVP_KN_VLIB_CANNY_NONMAX_SUPPRESSION:
                {
                    DVP_CannyNonMaxSuppression_t *pCnonmax = dvp_knode_to(&pNodes[n], DVP_CannyNonMaxSuppression_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inMag, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inGradX, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->inGradY, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCnonmax->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_GRADIENT_V5x5_PYRAMID_8:
                case DVP_KN_VLIB_GRADIENT_H5x5_PYRAMID_8:
                case DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_8:
                case DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_16:
                {
                    DVP_Gradient_t *pGrad = dvp_knode_to(&pNodes[n], DVP_Gradient_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->scratch, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pGrad->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_CANNY_HYST_THRESHHOLD:
                case DVP_KN_VLIB_CANNY_HYST_THRESHHOLD:
                case DVP_KN_VLIB_HYST_THRESHOLD:
                {
                    DVP_CannyHystThresholding_t *pDth = dvp_knode_to(&pNodes[n], DVP_CannyHystThresholding_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inMag, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inEdgeMap, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_NONMAXSUPPRESS_3x3_S16:
                case DVP_KN_NONMAXSUPPRESS_5x5_S16:
                case DVP_KN_NONMAXSUPPRESS_7x7_S16:
                case DVP_KN_VLIB_NONMAXSUPPRESS_3x3_S16:
                case DVP_KN_VLIB_NONMAXSUPPRESS_5x5_S16:
                case DVP_KN_VLIB_NONMAXSUPPRESS_7x7_S16:
                {
                    DVP_Threshold_t *pNonmax = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pNonmax->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pNonmax->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_INIT_HISTOGRAM_8:
                case DVP_KN_VLIB_INIT_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->edges, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_HISTOGRAM_8:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->edges, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[0], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[1], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[2], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_WEIGHTED_HISTOGRAM_8:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->edges, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->binWeights, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[0], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[1], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[2], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->edges, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[0], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_WEIGHTED_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->edges, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->binWeights, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[0], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_HISTOGRAM_ND_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->numBinsArray, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[0], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[1], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->h[2], DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_TRUE);
                    DVP_PrintImage(DVP_ZONE_KGM, &pH->input);
                    break;
                }
                case DVP_KN_VLIB_BHATTACHAYA_DISTANCE:
                {
                    DVP_Distance_t *pDist = dvp_knode_to(&pNodes[n], DVP_Distance_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDist->X, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDist->Y, DVP_FALSE);
                    break;
                }
                case DVP_KN_VLIB_L1DISTANCE:
                {
                    DVP_Distance_t *pDist = dvp_knode_to(&pNodes[n], DVP_Distance_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDist->X, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDist->Y, DVP_FALSE);
                    break;
                }
                case DVP_KN_VLIB_INIT_VAR_CONST_S16:
                case DVP_KN_VLIB_INIT_VAR_CONST_S32:
                {
                    DVP_ConstVar_t *pV = dvp_knode_to(&pNodes[n], DVP_ConstVar_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pV->var, DVP_FALSE);
                    break;
                }
                case DVP_KN_VLIB_DISPARITY_SAD8:
                case DVP_KN_VLIB_DISPARITY_SAD16:
                {
                    DVP_Disparity_t *pT = dvp_knode_to(&pNodes[n], DVP_Disparity_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->left, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->right, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->cost, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->mincost, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->disparity, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_SUB_BACK_S16:
                case DVP_KN_VLIB_SUB_BACK_S32:
                {
                    DVP_SubBackground_t *pSB = dvp_knode_to(&pNodes[n], DVP_SubBackground_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pSB->newestData, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pSB->currentMean, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pSB->currentVar, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pSB->output32packed, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_MOG_S16:
                case DVP_KN_VLIB_MOG_S32:
                {
                    DVP_Mog_t *pMOG = dvp_knode_to(&pNodes[n], DVP_Mog_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->image, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->wts, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->means, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->vars, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->compIndex, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->scratch, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pMOG->fgmask, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_NORMALFLOW_16:
                {
                    DVP_NormalFlow_t *pNF = dvp_knode_to(&pNodes[n], DVP_NormalFlow_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->mag, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->x, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->y, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->lut, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->u, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pNF->v, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_16:
                case DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_3D:
                {
                    DVP_Simplex_t *pS = dvp_knode_to(&pNodes[n], DVP_Simplex_t);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->start, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->step, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->stop, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[0], DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[1], DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[2], DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[3], DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[4], DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pS->intBuf[5], DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_HARRIS_SCORE_7x7:
                case DVP_KN_VLIB_HARRIS_SCORE_7x7_U32:
                {
                    DVP_HarrisDSP_t *pHs = dvp_knode_to(&pNodes[n], DVP_HarrisDSP_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pHs->inGradX, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pHs->inGradY, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHs->scratch, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pHs->harrisScore, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_TRACK_FEATURES_LUCAS_7x7:
                {
                    DVP_TrackFeatures_t *pLk = dvp_knode_to(&pNodes[n], DVP_TrackFeatures_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLk->im1, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLk->im2, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLk->gradx, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLk->grady, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLk->x, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLk->y, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLk->outx, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLk->outy, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLk->scratch, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_HOUGH_LINE_FROM_LIST:
                {
                    DVP_HoughLine_t *pHl = dvp_knode_to(&pNodes[n], DVP_HoughLine_t);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->pEdgeMapList, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->pSIN, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->pCOS, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->ping, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->pong, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->pang, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pHl->peng, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pHl->pOutHoughSpace, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_EWR_MEAN_S16:
                case DVP_KN_VLIB_EWR_MEAN_S32:
                {
                    DVP_EMean_t *pEWR = dvp_knode_to(&pNodes[n], DVP_EMean_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->mean, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->data, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->mask, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_EWR_VAR_S16:
                case DVP_KN_VLIB_EWR_VAR_S32:
                {
                    DVP_EMean_t *pEWR = dvp_knode_to(&pNodes[n], DVP_EMean_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->var, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->mean, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->data, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pEWR->mask, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_UWR_MEAN_S16:
                {
                    DVP_UMean_t *pUWR = dvp_knode_to(&pNodes[n], DVP_UMean_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_mean, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_data, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_data, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_mask, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_mask, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_mean, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_UWR_VAR_S16:
                {
                    DVP_UMean_t *pUWR = dvp_knode_to(&pNodes[n], DVP_UMean_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_var, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_mean, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_mean, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_data, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_data, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_mask, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->old_mask, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pUWR->new_var, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_LEGENDRE_MOMENTS_INIT:
                {
                    DVP_LegendreMoments_t *pLgm = dvp_knode_to(&pNodes[n], DVP_LegendreMoments_t);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pLgm->lPoly, DVP_FALSE);
                    break;
                }
                case DVP_KN_VLIB_LEGENDRE_MOMENTS:
                {
                    DVP_LegendreMoments_t *pLgm = dvp_knode_to(&pNodes[n], DVP_LegendreMoments_t);
                    dvp_rpc_return_image(rpc, DVP_CORE_DSP, &pLgm->im, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pLgm->lPoly, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pLgm->lMoments, DVP_TRUE);
                    break;
                }
                case DVP_KN_UYVY_TO_HSLp: // multiplane output
                case DVP_KN_VLIB_UYVY_TO_HSLp:
                {
                    DVP_Int2Pl_t *pLAB = dvp_knode_to(&pNodes[n], DVP_Int2Pl_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output1, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output2, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output3, DVP_TRUE);
                    break;
                }
                case DVP_KN_UYVY_TO_LABp:
                case DVP_KN_VLIB_UYVY_TO_LABp:
                {
                    DVP_Int2Pl_t *pLAB = dvp_knode_to(&pNodes[n], DVP_Int2Pl_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output1, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output2, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output3, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pLAB->scratch, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_NONMAXSUPPRESS_U32:
                {
                    DVP_Nonmax_NxN_t *pNonmax32 = dvp_knode_to(&pNodes[n], DVP_Nonmax_NxN_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pNonmax32->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pNonmax32->scratch, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pNonmax32->pixIndex, DVP_TRUE);
                    break;
                }
                case DVP_KN_VLIB_KALMAN_2x4:
                case DVP_KN_VLIB_KALMAN_4x6:
                {
                    DVP_KalmanFilter_t *pKf = dvp_knode_to(&pNodes[n], DVP_KalmanFilter_t);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->inMeasurements, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->transition, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->errorCov, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->predictedErrorCov, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->state, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->predictedState, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->measurement, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->processNoiseCov, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->measurementNoiseCov, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->kalmanGain, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->temp1, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->temp2, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pKf->temp3, DVP_TRUE);
                    break;
                }
#endif
#ifdef DVP_USE_TISMO
                case DVP_KN_TISMO_DISPARITY:
                {
                    DVP_Tismo_t *pT = dvp_knode_to(&pNodes[n], DVP_Tismo_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->left, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->right, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->output, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->invalid, DVP_TRUE);
                    break;
                }
#endif
#ifdef DVP_USE_TISMOV02
                case DVP_KN_TISMOV02_DISPARITY:
                {
                    DVP_TismoV02_t *pT = dvp_knode_to(&pNodes[n], DVP_TismoV02_t);

                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->in_left, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->in_right, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->in_leftIntegralImage, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->in_rightIntegralImage, DVP_FALSE);

                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->out_raw, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->out_invalid, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->out_confidence, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->out_matchScore, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pT->out_lumaU8, DVP_TRUE);
                    break;
                }
#endif
#ifdef DVP_USE_ORB
                case DVP_KN_ORB:
                {
                    DVP_Orb_t * pOrb = dvp_knode_to(&pNodes[n], DVP_Orb_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pOrb->inputImage, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pOrb->filtImage,  DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pOrb->harrisImage, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pOrb->integralImage, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pOrb->orbPattern31, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pOrb->orbOutput.orbData, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pOrb->orbOutput.outImage, DVP_TRUE);
                    break;
                }
#endif
#ifdef DVP_USE_DEI
                case DVP_KN_DEI_DEINTERLACER:
                {
                    DVP_Deinterlacer_t *pD = dvp_knode_to(&pNodes[n], DVP_Deinterlacer_t);

                    if (pD->phy_virt_flag == 0) {
                        dvp_rpc_return_image (rpc, DVP_GetSupportedRemoteCore(),  &pD->phy_fld_in_current, DVP_TRUE);
                        dvp_rpc_return_image (rpc, DVP_GetSupportedRemoteCore(),  &pD->phy_fld_in_prev, DVP_TRUE);
                    }

                    dvp_rpc_return_buffer (rpc, DVP_GetSupportedRemoteCore(), &pD->phy_luma_d0, DVP_TRUE);
                    dvp_rpc_return_buffer (rpc, DVP_GetSupportedRemoteCore(), &pD->phy_luma_d1, DVP_TRUE);
                    dvp_rpc_return_buffer (rpc, DVP_GetSupportedRemoteCore(), &pD->phy_luma_d2, DVP_TRUE);
                    break;
                }
#endif
#ifdef DVP_USE_RVM
                case DVP_KN_RVM:
                {
                    DVP_Rvm_t * pRvm = dvp_knode_to(&pNodes[n], DVP_Rvm_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pRvm->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pRvm->output, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pRvm->lut, DVP_FALSE);
                    break;
                }
#endif
#ifdef DVP_USE_IMGLIB
                case DVP_KN_IMG_BOUNDARY_8:
                case DVP_KN_IMG_BOUNDARY_16s:
                {
                    DVP_Boundary_t *pBoundary = dvp_knode_to(&pNodes[n], DVP_Boundary_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pBoundary->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pBoundary->outCoord, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pBoundary->outGray, DVP_TRUE);
                    break;
                }
                case DVP_KN_YUV422p_TO_RGB565:
                case DVP_KN_SOBEL_3x3_8:
                case DVP_KN_IMG_YUV422p_TO_RGB565:
                case DVP_KN_IMG_SOBEL_3x3_8:
                case DVP_KN_IMG_SOBEL_3x3_16s:
                case DVP_KN_IMG_SOBEL_5x5_16s:
                case DVP_KN_IMG_SOBEL_7x7_16s:
                case DVP_KN_IMG_PIX_SAT:
                case DVP_KN_IMG_PIX_EXPAND:
                case DVP_KN_IMG_PERIMETER_8:
                case DVP_KN_IMG_PERIMETER_16:
                case DVP_KN_IMG_MEDIAN_3x3_8:
                case DVP_KN_IMG_MEDIAN_3x3_16s:
                case DVP_KN_IMG_FDCT_8x8:
                case DVP_KN_IMG_IDCT_8x8_12Q4:
                {
                    DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pIO->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_IMG_CLIPPING_16:
                {
                    DVP_CannyHystThresholding_t *pDth = dvp_knode_to(&pNodes[n], DVP_CannyHystThresholding_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->inMag, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pDth->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_IMG_HISTOGRAM_8:
                case DVP_KN_IMG_HISTOGRAM_16:
                {
                    DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pH->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->histArray, DVP_TRUE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pH->hOut, DVP_TRUE);
                    break;
                }
                case DVP_KN_CANNY_IMAGE_SMOOTHING:
                case DVP_KN_CONV_3x3:
                case DVP_KN_CONV_5x5:
                case DVP_KN_CONV_7x7:
                case DVP_KN_IMG_CONV_3x3:
                case DVP_KN_IMG_CONV_5x5:
                case DVP_KN_IMG_CONV_7x7:
                case DVP_KN_IMG_CONV_11x11:
                case DVP_KN_IMG_CONV_3x3_I16s_C16:
                case DVP_KN_IMG_CONV_7x7_I16s_C16:
                case DVP_KN_IMG_CONV_11x11_I16s_C16:
                {
                    DVP_ImageConvolution_t *pImg = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->output, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->mask, DVP_FALSE);
                    break;
                }
                case DVP_KN_IMG_SAD_3x3:
                case DVP_KN_IMG_SAD_5x5:
                case DVP_KN_IMG_SAD_7x7:
                case DVP_KN_IMG_SAD_8x8:
                case DVP_KN_IMG_SAD_16x16:
                case DVP_KN_IMG_MAD_8x8:
                case DVP_KN_IMG_MAD_16x16:
                {
                    DVP_SAD_t *pImg = dvp_knode_to(&pNodes[n], DVP_SAD_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->output, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->refImg, DVP_FALSE);
                    break;
                }
                case DVP_KN_IMG_CONV_5x5_I8_C16:
                case DVP_KN_IMG_CONV_5x5_I16s_C16:
                case DVP_KN_IMG_CONV_7x7_I8_C16:
                {
                    DVP_ImageConvolution_with_buffer_t *pImg_buff = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_with_buffer_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg_buff->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg_buff->mask, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pImg_buff->scratch, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg_buff->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_THR_GT2MAX_8:
                case DVP_KN_THR_GT2MAX_16:
                case DVP_KN_THR_GT2THR_8:
                case DVP_KN_THR_GT2THR_16:
                case DVP_KN_THR_LE2MIN_8:
                case DVP_KN_THR_LE2MIN_16:
                case DVP_KN_THR_LE2THR_8:
                case DVP_KN_THR_LE2THR_16:
                case DVP_KN_IMG_THR_GT2MAX_8:
                case DVP_KN_IMG_THR_GT2MAX_16:
                case DVP_KN_IMG_THR_GT2THR_8:
                case DVP_KN_IMG_THR_GT2THR_16:
                case DVP_KN_IMG_THR_LE2MIN_8:
                case DVP_KN_IMG_THR_LE2MIN_16:
                case DVP_KN_IMG_THR_LE2THR_8:
                case DVP_KN_IMG_THR_LE2THR_16:
                case DVP_KN_IMG_ERRDIFF_BIN_8:
                case DVP_KN_IMG_ERRDIFF_BIN_16:
                {
                    DVP_Threshold_t *pImg = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pImg->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_IMG_YC_DEMUX_BE16_8:
                case DVP_KN_IMG_YC_DEMUX_LE16_8:
                {
                    DVP_Int2Pl_t *pLAB = dvp_knode_to(&pNodes[n], DVP_Int2Pl_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output1, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output2, DVP_TRUE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pLAB->output3, DVP_TRUE);
                    break;
                }
                case DVP_KN_IMG_CORR_3x3:
                case DVP_KN_IMG_CORR_3x3_I8_C16s:
                case DVP_KN_IMG_CORR_3x3_I16s_C16s:
                case DVP_KN_IMG_CORR_5x5_I16s_C16s:
                case DVP_KN_IMG_CORR_11x11_I8_C16s:
                case DVP_KN_IMG_CORR_11x11_I16s_C16s:
                case DVP_KN_IMG_CORR_GEN_I16s_C16s:
                case DVP_KN_IMG_CORR_GEN_IQ:
                {
                    DVP_ImageCorrelation_t *pCOR = dvp_knode_to(&pNodes[n], DVP_ImageCorrelation_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCOR->input, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCOR->mask, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pCOR->output, DVP_TRUE);
                    break;
                }
                case DVP_KN_IMG_QUANTIZE_16:
                {
                    DVP_ImageQuantization_t *pQnt = dvp_knode_to(&pNodes[n], DVP_ImageQuantization_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pQnt->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pQnt->recip_tbl, DVP_FALSE);
                    break;
                }
                case DVP_KN_IMG_WAVE_HORZ:
                {
                    DVP_ImageWavelet_t *pWv = dvp_knode_to(&pNodes[n], DVP_ImageWavelet_t);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pWv->input, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pWv->qmf, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pWv->mqmf, DVP_FALSE);
                    dvp_rpc_return_image(rpc, DVP_GetSupportedRemoteCore(), &pWv->output, DVP_TRUE);
                    break;
                }
#endif

#ifdef DVP_USE_DSPLIB
                case DVP_KN_DSP_ADD16:
                case DVP_KN_DSP_ADD32:
                {
                    DVP_DSPFunc * pDSPData = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPData->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPData->input1, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPData->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_AUTOCORR16:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_BITEXP32:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    break;
                }
                case DVP_KN_DSP_BLKESWAP16:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_BLKESWAP32:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_BLKESWAP64:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_BLKMOVE:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_DOTPRODSQR:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_DOTPROD:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input1, DVP_FALSE);
                    break;
                }
                case DVP_KN_DSP_FFT_16x16:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_FFT_16x16R:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_FFT_16x16_IMRE:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_FFT_16x32:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_FFT_32x32:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_FFT_32x32s:
                {
                    DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->twoway, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pBuf->output0, DVP_TRUE);
                    break;
                }

                case DVP_KN_DSP_MUL:
                case DVP_KN_DSP_MUL_CPLX:
                {
                    DVP_DSP_MatMul* pDSPMult = dvp_knode_to(&pNodes[n], DVP_DSP_MatMul);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMult->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMult->input1, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMult->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_FIR_CPLX:
                case DVP_KN_DSP_FIR_HM4x4:
                case DVP_KN_DSP_FIR_GEN:
                case DVP_KN_DSP_FIR_HM17_RA8x8:
                case DVP_KN_DSP_FIR_R4:
                case DVP_KN_DSP_FIR_R8:
                case DVP_KN_DSP_FIR_HM16_RM8A8x8:
                case DVP_KN_DSP_FIR_SYM:
                {
                    DVP_DSPFunc* pDSPFIR = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPFIR->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPFIR->twoway, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPFIR->output0, DVP_TRUE);
                    break;
                }
                case DVP_KN_DSP_FIR_LMS2:
                {
                    DVP_DSPFunc* pDSPFIR_LMS2 = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPFIR_LMS2->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPFIR_LMS2->twoway, DVP_FALSE);
                    break;
                }
                case DVP_KN_DSP_IFFT_16x16:
                case DVP_KN_DSP_IFFT_16x16_IMRE:
                case DVP_KN_DSP_IFFT_16x32:
                case DVP_KN_DSP_IFFT_32x32:
                {
                    DVP_DSPFunc* pDSPIIR = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIIR->input0, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIIR->twoway, DVP_FALSE);
                    dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIIR->output0, DVP_TRUE);
                    break;
                }

        case DVP_KN_DSP_IIR:
        {
                    DVP_DSPFunc* pDSPIIR = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                    dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pDSPIIR->input0, DVP_FALSE);
           dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pDSPIIR->input1, DVP_FALSE);
           dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pDSPIIR->twoway, DVP_FALSE);
           dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pDSPIIR->output0, DVP_TRUE);
            dvp_rpc_return_buffer(rpc, DVP_CORE_DSP, &pDSPIIR->output1, DVP_TRUE);
                    break;
                }

        case DVP_KN_DSP_IIR_LAT:
        {
           DVP_DSPFunc* pDSPIIR_LAT = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
           dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIIR_LAT->input0, DVP_FALSE);
           dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIIR_LAT->input1, DVP_FALSE);
           dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIIR_LAT->twoway, DVP_FALSE);
           dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIIR_LAT->output0, DVP_TRUE);
                    break;
        }
        case DVP_KN_DSP_IIR_SS:
        {
                DVP_DSPFunc* pDSPIIR_SS = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIIR_SS->input0, DVP_FALSE);
                dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIIR_SS->input1, DVP_FALSE);
                break;
        }
        case DVP_KN_DSP_MAT_TRANS:
        {
            DVP_DSPFunc* pDSPMatTrans = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMatTrans->input0, DVP_FALSE);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMatTrans->output0, DVP_TRUE);
            break;
        }
        case DVP_KN_DSP_MAXIDX:
        case DVP_KN_DSP_MAXVAL:
        case DVP_KN_DSP_MINVAL:
        {
            DVP_DSPFunc* pDSPMaxIdx = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMaxIdx->input0, DVP_FALSE);
            break;
        }
        case DVP_KN_DSP_MINERROR:
        {
            DVP_DSPFunc* pDSPMinError = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMinError->input0, DVP_FALSE);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMinError->input1, DVP_FALSE);
            break;
        }
        case DVP_KN_DSP_MUL32:
        {
            DVP_DSPFunc* pDSPMult32 = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMult32->input0, DVP_FALSE);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMult32->input1, DVP_FALSE);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPMult32->output0, DVP_TRUE);
            break;
        }
        case DVP_KN_DSP_NEG32:
        {
            DVP_DSPFunc* pDSPNeg32 = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPNeg32->input0, DVP_FALSE);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPNeg32->output0, DVP_TRUE);
            break;
        }
        case DVP_KN_DSP_RECIP16:
        {
            DVP_DSPFunc* pDSPRecip16 = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPRecip16->input0, DVP_FALSE);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPRecip16->twoway, DVP_TRUE);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPRecip16->output0, DVP_TRUE);
            break;
        }
        case DVP_KN_DSP_VECSUMSQ:
        {
            DVP_DSPFunc* pDSPVecSum = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPVecSum->input0, DVP_FALSE);
            break;
        }
        case DVP_KN_DSP_W_VEC:
        {
            DVP_DSPFunc* pDSPW_Vec = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPW_Vec->input0, DVP_FALSE);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPW_Vec->input1, DVP_FALSE);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPW_Vec->output0, DVP_TRUE);
            break;
        }
        case DVP_KN_GEN_TWIDDLE_FFT_16X16:
        {
            DVP_DSPFunc* pDSPFFT16x16Twd = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPFFT16x16Twd->input0, DVP_FALSE);
            break;
        }
        case DVP_KN_GEN_TWIDDLE_FFT_16X16_IMRE:
        {
            DVP_DSPFunc* pDSPFFT16x16Twd = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPFFT16x16Twd->input0, DVP_FALSE);
            break;
        }
        case DVP_KN_GEN_TWIDDLE_FFT_16X32:
        {
            DVP_DSPFunc* pDSPFFT16x32Twd = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPFFT16x32Twd->input0, DVP_FALSE);
            break;
        }
        case DVP_KN_GEN_TWIDDLE_FFT_32x32:
        {
            DVP_DSPFunc* pDSPFFT32x32Twd = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPFFT32x32Twd->input0, DVP_FALSE);
            break;
        }
        case DVP_KN_GEN_TWIDDLE_IFFT_16X16:
        {
            DVP_DSPFunc* pDSPIFFT16x16Twd = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIFFT16x16Twd->input0, DVP_FALSE);
            break;
        }
        case DVP_KN_GEN_TWIDDLE_IFFT_16X16_IMRE:
        {
            DVP_DSPFunc* pDSPIFFT16x16Twd = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
            dvp_rpc_return_buffer(rpc, DVP_GetSupportedRemoteCore(), &pDSPIFFT16x16Twd->input0, DVP_FALSE);
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

static thread_ret_t DVP_KernelGraphManagerThread_DSP(void *arg __attribute__((unused)))
{
    DVP_KGM_Thread_t kgmt;

    thread_nextaffinity();
    while (queue_read(workqueue, true_e, &kgmt) == true_e)
    {
        DVP_KernelNode_t *pSubNodes = kgmt.pSubNodes;
        DVP_U32 startNode = kgmt.startNode;
        DVP_U32 numNodes = kgmt.numNodes;
        DVP_U32 processed = 0;

        processed = DVP_KernelGraphManager_DSP(pSubNodes, startNode, numNodes);

        kgmt.numNodesExecuted = processed;
        queue_write(retqueue, true_e, &kgmt);
    }
    thread_exit(0);
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

MODULE_EXPORT DVP_U32 DVP_KernelGraphManagerInit(DVP_RPC_t *pRPC, DVP_RPC_Core_t *pCore)
{
    if (rpc == NULL && core == NULL && pRPC != NULL && pCore != NULL)
    {
        int status;
        DVP_U32 version = DVP_VERSION;
        DVP_U16 width = 320;
        DVP_U16 height = 240;
        DVP_RPC_Parameter_t params[] = {
            {DVP_RPC_READ_ONLY, sizeof(DVP_U16), &width, NULL},
            {DVP_RPC_READ_ONLY, sizeof(DVP_U16), &height, NULL},
            {DVP_RPC_READ_ONLY, sizeof(DVP_U32), &version, NULL},
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
            worker = thread_create(DVP_KernelGraphManagerThread_DSP, NULL);
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
        return DVP_KernelGraphManager_DSP(pSubNodes, startNode, numNodes);
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
#ifdef DVP_USE_VLIB
            case DVP_KN_XYXY_TO_Y800: // single planes
            case DVP_KN_VLIB_XYXY_TO_Y800:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_UYVY, FOURCC_VYUY};
                if (DVP_Image_Validate(&pIO->input, 2, 2, 16, 1, &valid_colors[0], 2) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 8, 8, 16, 1, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_RGBp: // multiplane output
            case DVP_KN_VLIB_UYVY_TO_RGBp:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_RGBP};
                if (DVP_Image_Validate(&pIO->input, 2, 2, 8, 1, colorUYVY, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 8, 8, 8, 1, valid_colors, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_YUV420p:
            case DVP_KN_VLIB_UYVY_TO_YUV420p:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_IYUV, FOURCC_YV12};
                if (DVP_Image_Validate(&pIO->input, 2, 2, 8, 2, colorUYVY, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 8, 8, 8, 2, &valid_colors[0], 2) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_YUV422p:
            case DVP_KN_VLIB_UYVY_TO_YUV422p:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_YV16, FOURCC_YU16};
                if (DVP_Image_Validate(&pIO->input, 2, 2, 16, 1, colorUYVY, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 8, 8, 16, 1, &valid_colors[0], 2) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YUV422p_TO_UYVY:
            case DVP_KN_VLIB_YUV422p_TO_UYVY:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_YV16, FOURCC_YU16};
                if (DVP_Image_Validate(&pIO->input, 2, 2, 16, 1, &valid_colors[0], 2) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 8, 8, 16, 1, colorUYVY, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_NV12_TO_YUV444p:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_NV12, FOURCC_YV24, FOURCC_YU24};
                if (DVP_Image_Validate(&pIO->input, 1, 1, 16, 1, &valid_colors[0], 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 8, 8, 16, 1, &valid_colors[1], 2) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_INTEGRAL_IMAGE_8:
            case DVP_KN_VLIB_INTEGRAL_IMAGE_8:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 4, 4, 8, 1, colorY32, 1) == DVP_FALSE || //Writes using EDMA
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_INIT_MEAN_LUMA_S32:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 4, 4, 1, 1, colorY32, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_INTEGRAL_IMAGE_16:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 2, 2, 4, 1, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 4, 4, 4, 1, colorY32, 1) == DVP_FALSE || //Writes using EDMA
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_PACK_MASK_32:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_BIN1};
                if (DVP_Image_Validate(&pIO->input, 1, 1, 32, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 4, 4, 32, 1, &valid_colors[0], 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_UNPACK_MASK_32:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_BIN1};
                if (DVP_Image_Validate(&pIO->input, 1, 1, 8, 1, &valid_colors[0], 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_IMAGE_PYRAMID_8:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 8, 8, 8, 8, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 8, 8, 8, 8, colorY800, 1) == DVP_FALSE)
                    // TODO: Add output buffer size constraint
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_IMAGE_PYRAMID_16:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 8, 8, 8, 8, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 8, 8, 8, 8, colorY16, 1) == DVP_FALSE)
                    // TODO: Add output buffer size constraint
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_INIT_MEAN_LUMA_S16:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_EXTRACT_BACK_8_16:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 2, 2, 8, 1, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DILATE_CROSS:
            case DVP_KN_DILATE_SQUARE:
            case DVP_KN_DILATE_MASK:
            case DVP_KN_ERODE_CROSS:
            case DVP_KN_ERODE_MASK:
            case DVP_KN_ERODE_SQUARE:
            case DVP_KN_VLIB_DILATE_CROSS:
            case DVP_KN_VLIB_DILATE_SQUARE:
            case DVP_KN_VLIB_DILATE_MASK:
            case DVP_KN_VLIB_ERODE_CROSS:
            case DVP_KN_VLIB_ERODE_MASK:
            case DVP_KN_VLIB_ERODE_SQUARE:
            case DVP_KN_VLIB_ERODE_SINGLEPIXEL:
            {
                DVP_Morphology_t *pMorph = dvp_knode_to(&pNodes[n], DVP_Morphology_t);
                if (DVP_Image_Validate(&pMorph->input, 8, 8, 32, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pMorph->output, 8, 8, 32, 1, colorY800, 1) == DVP_FALSE ||
                    pMorph->input.width  > pMorph->output.width ||
                    pMorph->input.height > pMorph->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (pNodes[n].header.kernel == DVP_KN_ERODE_MASK ||
                    pNodes[n].header.kernel == DVP_KN_DILATE_MASK ||
                    pNodes[n].header.kernel == DVP_KN_VLIB_ERODE_MASK ||
                    pNodes[n].header.kernel == DVP_KN_VLIB_DILATE_MASK)
                {
                    if (DVP_Image_Validate(&pMorph->mask, 8, 1, 1, 1, colorY800, 1) == DVP_FALSE)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                }
                break;
            }
            case DVP_KN_IIR_HORZ:
            case DVP_KN_IIR_VERT:
            case DVP_KN_VLIB_IIR_HORZ:
            case DVP_KN_VLIB_IIR_VERT:
            {
                DVP_IIR_t *pIIR = dvp_knode_to(&pNodes[n], DVP_IIR_t);
                if (DVP_Image_Validate(&pIIR->input, 1, 1, 4, 4, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIIR->output, 4, 4, 4, 4, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIIR->scratch, 4, 4, 4, 4, colorY800, 1) == DVP_FALSE ||
                    pIIR->input.width  > pIIR->output.width ||
                    pIIR->input.height > pIIR->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_IIR_HORZ_16:
            case DVP_KN_VLIB_IIR_VERT_16:
            {
                DVP_IIR_t *pIIR = dvp_knode_to(&pNodes[n], DVP_IIR_t);
                if (DVP_Image_Validate(&pIIR->input, 2, 2, 4, 4, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIIR->output, 4, 4, 4, 4, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIIR->scratch, 4, 4, 4, 4, colorY16, 1) == DVP_FALSE ||
                    pIIR->input.width  > pIIR->output.width ||
                    pIIR->input.height > pIIR->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_CCL:
            {
                DVP_CCL_t *pParam = dvp_knode_to(&pNodes[n], DVP_CCL_t);
                if (DVP_Image_Validate(&pParam->input, 1, 1, 32, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pParam->output, 8, 8, 32, 1, colorY800, 1) == DVP_FALSE ||
                    pParam->input.width > pParam->output.width ||
                    pParam->input.height > pParam->output.height ||
                    (pParam->connected8Flag != DVP_CCL_4_CONNECTED &&
                     pParam->connected8Flag != DVP_CCL_8_CONNECTED))
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_CANNY_2D_GRADIENT:
            case DVP_KN_VLIB_CANNY_2D_GRADIENT:
            {
                DVP_Canny2dGradient_t *pD = dvp_knode_to(&pNodes[n], DVP_Canny2dGradient_t);
                if (DVP_Image_Validate(&pD->input, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pD->outGradX, 2, 2, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pD->outGradY, 2, 2, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pD->outMag, 2, 2, 8, 1, colorY16, 1) == DVP_FALSE ||
                    pD->input.width > pD->outGradX.width ||
                    pD->input.width > pD->outGradY.width ||
                    pD->input.width > pD->outMag.width ||
                    pD->input.height > pD->outGradX.height ||
                    pD->input.height > pD->outGradY.height ||
                    pD->input.height > pD->outMag.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_CANNY_NONMAX_SUPPRESSION:
            case DVP_KN_VLIB_CANNY_NONMAX_SUPPRESSION:
            {
                DVP_CannyNonMaxSuppression_t *pCnonmax = dvp_knode_to(&pNodes[n], DVP_CannyNonMaxSuppression_t);
                if (DVP_Image_Validate(&pCnonmax->inMag, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pCnonmax->inGradX, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCnonmax->inGradY, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCnonmax->output, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    pCnonmax->output.width  < pCnonmax->inMag.width ||
                    pCnonmax->output.height < pCnonmax->inMag.height ||
                    pCnonmax->output.width  < pCnonmax->inGradX.width ||
                    pCnonmax->output.height < pCnonmax->inGradX.height ||
                    pCnonmax->output.width  < pCnonmax->inGradY.width ||
                    pCnonmax->output.height < pCnonmax->inGradY.height ||
                    pCnonmax->inMag.width > 720)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_GRADIENT_V5x5_PYRAMID_8:
            case DVP_KN_VLIB_GRADIENT_H5x5_PYRAMID_8:
            case DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_8:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 1, 1, 2, 2, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 1, 1, 2, 2, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width  > (pIO->output.width*2) ||
                    pIO->input.height > (pIO->output.height*2))
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_GAUSSIAN_5x5_PYRAMID_16:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 2, 2, 2, 2, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 2, 2, 2, 2, colorY16, 1) == DVP_FALSE ||
                    pIO->input.width  > (pIO->output.width*2) ||
                    pIO->input.height > (pIO->output.height*2))
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_CANNY_HYST_THRESHHOLD:
            case DVP_KN_VLIB_CANNY_HYST_THRESHHOLD:
            {
                DVP_CannyHystThresholding_t *pDth = dvp_knode_to(&pNodes[n], DVP_CannyHystThresholding_t);
                if (DVP_Image_Validate(&pDth->inMag, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pDth->inEdgeMap, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pDth->output, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    pDth->output.width  < pDth->inMag.width ||
                    pDth->output.height < pDth->inMag.height ||
                    pDth->output.width  < pDth->inEdgeMap.width ||
                    pDth->output.height < pDth->inEdgeMap.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_HYST_THRESHOLD:
            {
                DVP_CannyHystThresholding_t *pDth = dvp_knode_to(&pNodes[n], DVP_CannyHystThresholding_t);
                if (DVP_Image_Validate(&pDth->inMag, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pDth->inEdgeMap, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pDth->output, 4, 4, 1, 1, colorY32, 1) == DVP_FALSE ||
                    pDth->output.width  < pDth->inMag.width ||
                    pDth->output.height < pDth->inMag.height ||
                    pDth->output.width  < pDth->inEdgeMap.width ||
                    pDth->output.height < pDth->inEdgeMap.height ||
                    pDth->inMag.width > 720 ||
                    pDth->inMag.height > 480)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_NONMAXSUPPRESS_3x3_S16:
            case DVP_KN_NONMAXSUPPRESS_5x5_S16:
            case DVP_KN_NONMAXSUPPRESS_7x7_S16:
            case DVP_KN_VLIB_NONMAXSUPPRESS_3x3_S16:
            case DVP_KN_VLIB_NONMAXSUPPRESS_5x5_S16:
            case DVP_KN_VLIB_NONMAXSUPPRESS_7x7_S16:
            {
                DVP_Threshold_t *pThresh = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                if (DVP_Image_Validate(&pThresh->input, 2, 2, 8, 1, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pThresh->output, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE ||
                    pThresh->input.width  > pThresh->output.width ||
                    pThresh->input.height > pThresh->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_INIT_HISTOGRAM_8:
            case DVP_KN_VLIB_INIT_HISTOGRAM_16:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                if (DVP_Buffer_Validate(&pH->edges) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->histArray) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_HISTOGRAM_8:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                if (DVP_Image_Validate(&pH->input, 1, 1, 4, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Buffer_Validate(&pH->edges) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->histArray) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[0]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[1]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[2]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->hOut) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_WEIGHTED_HISTOGRAM_8:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                if (DVP_Image_Validate(&pH->input, 1, 1, 4, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Buffer_Validate(&pH->edges) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->histArray) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->binWeights) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[0]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[1]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[2]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->hOut) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_HISTOGRAM_16:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                if (DVP_Image_Validate(&pH->input, 1, 1, 4, 1, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Buffer_Validate(&pH->edges) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->histArray) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[0]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->hOut) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_WEIGHTED_HISTOGRAM_16:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                if (DVP_Image_Validate(&pH->input, 1, 1, 4, 1, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Buffer_Validate(&pH->edges) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->histArray) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->binWeights) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[0]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->hOut) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_HISTOGRAM_ND_16:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                if (DVP_Image_Validate(&pH->input, 4, 4, 4, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->numBinsArray) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[0]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[1]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->h[2]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pH->hOut) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_BHATTACHAYA_DISTANCE:
            case DVP_KN_VLIB_L1DISTANCE:
            {
                DVP_Distance_t *pD = dvp_knode_to(&pNodes[n], DVP_Distance_t);
                if (DVP_Image_Validate(&pD->X, 2, 2, 4, 1, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pD->Y, 2, 2, 4, 1, colorY16, 1) == DVP_FALSE)   //Reads using EDMA
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_INIT_VAR_CONST_S16:
            {
                DVP_ConstVar_t *pParam = dvp_knode_to(&pNodes[n], DVP_ConstVar_t);
                if (DVP_Image_Validate(&pParam->var, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_INIT_VAR_CONST_S32:
            {
                DVP_ConstVar_t *pParam = dvp_knode_to(&pNodes[n], DVP_ConstVar_t);
                if (DVP_Image_Validate(&pParam->var, 4, 4, 1, 1, colorY32, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_DISPARITY_SAD8:
            {
                DVP_Disparity_t *pT = dvp_knode_to(&pNodes[n], DVP_Disparity_t);
                if (DVP_Image_Validate(&pT->left, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->right, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->cost, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->mincost, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->disparity, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_DISPARITY_SAD16:
            {
                DVP_Disparity_t *pT = dvp_knode_to(&pNodes[n], DVP_Disparity_t);
                if (DVP_Image_Validate(&pT->left, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->right, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->cost, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->mincost, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->disparity, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_SUB_BACK_S16:
            {
                DVP_SubBackground_t *pParam = dvp_knode_to(&pNodes[n], DVP_SubBackground_t);
                if (DVP_Image_Validate(&pParam->output32packed, 8, 8, 8, 1, colorY32, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->newestData, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pParam->currentMean, 2, 2, 8, 1, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pParam->currentVar, 2, 2, 8, 1, colorY16, 1) == DVP_FALSE)    //Reads using EDMA
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_SUB_BACK_S32:
            {
                DVP_SubBackground_t *pParam = dvp_knode_to(&pNodes[n], DVP_SubBackground_t);
                if (DVP_Image_Validate(&pParam->output32packed, 8, 8, 4, 1, colorY32, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->newestData, 1, 1, 4, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pParam->currentMean, 4, 4, 4, 1, colorY32, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pParam->currentVar, 4, 4, 4, 1, colorY32, 1) == DVP_FALSE)    //Reads using EDMA
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_MOG_S16:
            {
                DVP_Mog_t *pParam = dvp_knode_to(&pNodes[n], DVP_Mog_t);
                if (DVP_Image_Validate(&pParam->image, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->wts, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->means, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->vars, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->scratch, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->compIndex, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->fgmask, 8, 8, 8, 1, colorY32, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_MOG_S32:
            {
                DVP_Mog_t *pParam = dvp_knode_to(&pNodes[n], DVP_Mog_t);
                if (DVP_Image_Validate(&pParam->image, 8, 8, 8, 1,colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->wts, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->means, 8, 8, 8, 1, colorY32, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->vars, 8, 8, 8, 1, colorY32, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->scratch, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->compIndex, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->fgmask, 8, 8, 8, 1, colorY32, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_NORMALFLOW_16:
            {
                DVP_NormalFlow_t *pN = dvp_knode_to(&pNodes[n], DVP_NormalFlow_t);
                if (DVP_Image_Validate(&pN->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pN->x, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->y, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->u, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->v, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->lut, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pN->mag, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_16:
            case DVP_KN_VLIB_NEDLER_MEAD_SIMPLEX_3D:
            {
                DVP_Simplex_t *pSmx = dvp_knode_to(&pNodes[n], DVP_Simplex_t);
                if (DVP_Buffer_Validate(&pSmx->start) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->step) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->stop) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[0]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[1]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[2]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[3]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[4]) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pSmx->intBuf[5]) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_HARRIS_SCORE_7x7:
            {
                DVP_HarrisDSP_t *pParam = dvp_knode_to(&pNodes[n], DVP_HarrisDSP_t);
                if (DVP_Image_Validate(&pParam->inGradX, 1, 1, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->inGradY, 1, 1, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->harrisScore, 1, 1, 1, 1, colorY16, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_HARRIS_SCORE_7x7_U32:
            {
                DVP_HarrisDSP_t *pParam = dvp_knode_to(&pNodes[n], DVP_HarrisDSP_t);
                if (DVP_Image_Validate(&pParam->inGradX, 8, 8, 4, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->inGradY, 8, 8, 4, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->harrisScore, 8, 8, 4, 1, colorY32, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_TRACK_FEATURES_LUCAS_7x7:
            {
                DVP_TrackFeatures_t *pTF = dvp_knode_to(&pNodes[n], DVP_TrackFeatures_t);
                if (DVP_Image_Validate(&pTF->im1, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pTF->im2, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pTF->gradx, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pTF->grady, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pTF->x) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pTF->y) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pTF->outx) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pTF->outy) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_HOUGH_LINE_FROM_LIST:
            {
                DVP_HoughLine_t *pParam = dvp_knode_to(&pNodes[n], DVP_HoughLine_t);
                if (DVP_Image_Validate(&pParam->pOutHoughSpace, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->pEdgeMapList) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->pSIN) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->pCOS) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->ping) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->pong) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->pang) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pParam->peng) == DVP_FALSE)
                    /*! \todo check more values from varaibles */
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_EWR_MEAN_S16:
            {
                DVP_EMean_t *pParam = dvp_knode_to(&pNodes[n], DVP_EMean_t);
                if (DVP_Image_Validate(&pParam->mean, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->mask, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pParam->data, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    pParam->mask.width != pParam->data.width/8)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_EWR_MEAN_S32:
            {
                DVP_EMean_t *pParam = dvp_knode_to(&pNodes[n], DVP_EMean_t);
                if (DVP_Image_Validate(&pParam->mean, 8, 8, 4, 1, colorY32, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->mask, 1, 1, 4, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pParam->data, 1, 1, 4, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    pParam->mask.width != pParam->data.width/8)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_EWR_VAR_S16:
            {
                DVP_EMean_t *pParam = dvp_knode_to(&pNodes[n], DVP_EMean_t);
                if (DVP_Image_Validate(&pParam->var, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->mean, 1, 1, 8, 1, colorY16, 1) == DVP_FALSE ||  //Reads using EDMA
                    DVP_Image_Validate(&pParam->mask, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pParam->data, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    pParam->mask.width != pParam->data.width/8)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_EWR_VAR_S32:
            {
                DVP_EMean_t *pParam = dvp_knode_to(&pNodes[n], DVP_EMean_t);
                if (DVP_Image_Validate(&pParam->var, 8, 8, 4, 1, colorY32, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->mean, 1, 1, 4, 1, colorY32, 1) == DVP_FALSE ||  //Reads using EDMA
                    DVP_Image_Validate(&pParam->mask, 1, 1, 4, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pParam->data, 1, 1, 4, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    pParam->mask.width != pParam->data.width/8)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_UWR_MEAN_S16:
            {
                DVP_UMean_t *pParam = dvp_knode_to(&pNodes[n], DVP_UMean_t);
                if (DVP_Image_Validate(&pParam->old_mean, 1, 1, 8, 1, colorY16, 1) == DVP_FALSE ||  //Reads using EDMA
                    DVP_Image_Validate(&pParam->old_mask, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->old_data, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pParam->new_mean, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->new_mask, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->new_data, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    pParam->old_mask.width != pParam->old_data.width/8 ||
                    pParam->new_mask.width != pParam->new_data.width/8)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_UWR_VAR_S16:
            {
                DVP_UMean_t *pParam = dvp_knode_to(&pNodes[n], DVP_UMean_t);
                if (DVP_Image_Validate(&pParam->old_var,  1, 1, 8, 1, colorY16, 1) == DVP_FALSE ||   //Reads using EDMA
                    DVP_Image_Validate(&pParam->old_mean, 1, 1, 8, 1, colorY16, 1) == DVP_FALSE ||   //Reads using EDMA
                    DVP_Image_Validate(&pParam->old_mask, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->old_data, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->new_var,  8, 8, 8, 1, colorY16, 1) == DVP_FALSE  ||
                    DVP_Image_Validate(&pParam->new_mean, 1, 1, 8, 1, colorY16, 1) == DVP_FALSE ||   //Reads using EDMA
                    DVP_Image_Validate(&pParam->new_mask, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->new_data, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    pParam->old_mask.width != pParam->old_data.width/8 ||
                    pParam->new_mask.width != pParam->new_data.width/8)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_LEGENDRE_MOMENTS_INIT:
            {
                DVP_LegendreMoments_t *pLgm = dvp_knode_to(&pNodes[n], DVP_LegendreMoments_t);
                if (DVP_Buffer_Validate(&pLgm->lPoly) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_LEGENDRE_MOMENTS:
            {
                DVP_LegendreMoments_t *pLgm = dvp_knode_to(&pNodes[n], DVP_LegendreMoments_t);
                if (DVP_Image_Validate(&pLgm->im, 8, 8, 8, 4, colorY16, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pLgm->lPoly) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pLgm->lMoments) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_HSLp: // multiplane output
            case DVP_KN_VLIB_UYVY_TO_HSLp: // multiplane output
            {
                DVP_Int2Pl_t *pParam = dvp_knode_to(&pNodes[n], DVP_Int2Pl_t);
                if (DVP_Image_Validate(&pParam->input, 1, 1, 8, 1, colorUYVY, 1) == DVP_FALSE ||  //Reads using EDMA
                    DVP_Image_Validate(&pParam->output1, 8, 8, 8, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output2, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output3, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    pParam->input.width  > pParam->output1.width ||
                    pParam->input.height > pParam->output1.height ||
                    pParam->input.width  > pParam->output2.width ||
                    pParam->input.height > pParam->output2.height ||
                    pParam->input.width  > pParam->output3.width ||
                    pParam->input.height > pParam->output3.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_UYVY_TO_LABp:
            case DVP_KN_VLIB_UYVY_TO_LABp:
            {
                DVP_Int2Pl_t *pParam = dvp_knode_to(&pNodes[n], DVP_Int2Pl_t);
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, colorUYVY, 1) == DVP_FALSE ||  //Reads using EDMA
                    DVP_Image_Validate(&pParam->output1, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output2, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output3, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    pParam->input.width  > pParam->output1.width ||
                    pParam->input.height > pParam->output1.height ||
                    pParam->input.width  > pParam->output2.width ||
                    pParam->input.height > pParam->output2.height ||
                    pParam->input.width  > pParam->output3.width ||
                    pParam->input.height > pParam->output3.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_NONMAXSUPPRESS_U32:
            {
                DVP_Nonmax_NxN_t *nonmax32 = dvp_knode_to(&pNodes[n], DVP_Nonmax_NxN_t);
                if (DVP_Image_Validate(&nonmax32->input, 8, 8, 8, 1, colorY32, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&nonmax32->pixIndex) == DVP_FALSE ||
                    DVP_Buffer_Validate(&nonmax32->scratch) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_VLIB_KALMAN_2x4:
            case DVP_KN_VLIB_KALMAN_4x6:
            {
                DVP_KalmanFilter_t *pKf = dvp_knode_to(&pNodes[n], DVP_KalmanFilter_t);
                if (DVP_Buffer_Validate(&pKf->inMeasurements) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->transition) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->errorCov) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->predictedErrorCov) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->state) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->predictedState) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->measurement) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->processNoiseCov) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->measurementNoiseCov) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->kalmanGain) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->temp1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->temp2) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pKf->temp3) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
#ifdef DVP_USE_TISMO
            case DVP_KN_TISMO_DISPARITY:
            {
                DVP_Tismo_t *pT = dvp_knode_to(&pNodes[n], DVP_Tismo_t);
                fourcc_t valid_colors[] = {FOURCC_Y16, FOURCC_NV12};
                if (DVP_Image_Validate(&pT->left, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->right, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->output, 1, 1, 1, 1, &valid_colors[0], 2) == DVP_FALSE ||
                    pT->left.width  != pT->right.width ||
                    pT->left.height != pT->right.height ||
                    pT->left.width  > pT->output.width ||
                    pT->left.height > pT->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                if (&pT->invalid)
                    if (DVP_Image_Validate(&pT->invalid, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                        pT->left.width  > pT->invalid.width ||
                        pT->left.height > pT->invalid.height)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
#ifdef DVP_USE_TISMOV02
            case DVP_KN_TISMOV02_DISPARITY:
            {
                DVP_TismoV02_t *pT = dvp_knode_to(&pNodes[n], DVP_TismoV02_t);
                fourcc_t valid_colors[] = {FOURCC_Y16, FOURCC_NV12};
                if (DVP_Image_Validate(&pT->in_left, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pT->in_right, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    pT->in_left.width  != pT->in_right.width ||
                    pT->in_left.height != pT->in_right.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (&pT->in_leftIntegralImage)
                    if (DVP_Image_Validate(&pT->in_leftIntegralImage, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                        pT->in_left.width  != pT->in_leftIntegralImage.width ||
                        pT->in_left.height != pT->in_leftIntegralImage.height)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (&pT->in_rightIntegralImage)
                    if (DVP_Image_Validate(&pT->in_rightIntegralImage, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                        pT->in_right.width  != pT->in_rightIntegralImage.width ||
                        pT->in_right.height != pT->in_rightIntegralImage.height)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (&pT->out_raw)
                    if (DVP_Image_Validate(&pT->out_raw, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                        pT->in_left.width  != pT->out_raw.width ||
                        pT->in_left.height != pT->out_raw.height)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (&pT->out_invalid)
                    if (DVP_Image_Validate(&pT->out_invalid, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                        pT->in_left.width  != pT->out_invalid.width ||
                        pT->in_left.height != pT->out_invalid.height)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (&pT->out_confidence)
                    if (DVP_Image_Validate(&pT->out_confidence, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                        pT->in_left.width  != pT->out_confidence.width ||
                        pT->in_left.height != pT->out_confidence.height)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (&pT->out_matchScore)
                    if (DVP_Image_Validate(&pT->out_matchScore, 1, 1, 1, 1, &valid_colors[0], 2) == DVP_FALSE ||
                        pT->in_left.width  != pT->out_matchScore.width ||
                        pT->in_left.height != pT->out_matchScore.height)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;

                if (&pT->out_lumaU8)
                    if (DVP_Image_Validate(&pT->out_lumaU8, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                        pT->in_left.width  != pT->out_lumaU8.width ||
                        pT->in_left.height != pT->out_lumaU8.height)
                        pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
#ifdef DVP_USE_ORB
            case DVP_KN_ORB:
            {
                DVP_Orb_t * pOrb = dvp_knode_to(&pNodes[n], DVP_Orb_t);
                fourcc_t valid_colors[] = {FOURCC_YU24};
                if (DVP_Image_Validate(&pOrb->inputImage, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pOrb->filtImage, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pOrb->harrisImage, 1, 1, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pOrb->integralImage, 1, 1, 1, 1, colorY32, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pOrb->orbPattern31) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pOrb->orbOutput.orbData) == DVP_FALSE ||
                    DVP_Image_Validate(&pOrb->orbOutput.outImage, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
#ifdef DVP_USE_DEI
            case DVP_KN_DEI_DEINTERLACER:
            {
                DVP_Deinterlacer_t *pD = dvp_knode_to(&pNodes[n], DVP_Deinterlacer_t);
                if (DVP_Image_Validate(&pD->phy_fld_in_current, 1, 1, 1, 1, colorUYVY, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pD->phy_fld_in_prev, 1, 1, 1, 1, colorUYVY, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pD->phy_luma_d0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pD->phy_luma_d1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pD->phy_luma_d2) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
#ifdef DVP_USE_RVM
            case DVP_KN_RVM:
            {
                DVP_Rvm_t *pParam = dvp_knode_to(&pNodes[n], DVP_Rvm_t);
                fourcc_t valid_colors[] = {FOURCC_Y800, FOURCC_UYVY};
                if (DVP_Image_Validate(&pParam->input, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output, 1, 1, 1, 1, valid_colors, dimof(valid_colors)) == DVP_FALSE ||
                    pParam->input.color != pParam->output.color ||
                    DVP_Buffer_Validate(&pParam->lut) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
#ifdef DVP_USE_IMGLIB
            case DVP_KN_IMG_BOUNDARY_8:
            {
                DVP_Boundary_t *pBoundary = dvp_knode_to(&pNodes[n], DVP_Boundary_t);
                if (DVP_Image_Validate(&pBoundary->input, 1, 1, 4, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Buffer_Validate(&pBoundary->outCoord) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBoundary->outGray) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_BOUNDARY_16s:
            {
                DVP_Boundary_t *pBoundary = dvp_knode_to(&pNodes[n], DVP_Boundary_t);
                if (DVP_Image_Validate(&pBoundary->input, 2, 2, 4, 1, colorY16, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Buffer_Validate(&pBoundary->outCoord) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBoundary->outGray) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_SOBEL_3x3_8:
            case DVP_KN_IMG_SOBEL_3x3_8:
            case DVP_KN_IMG_MEDIAN_3x3_8:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 1, 1, 8, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 8, 8, 8, 1, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_PERIMETER_8:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 1, 1, 16, 1, colorY800, 1) == DVP_FALSE || //Reads using EDMA
                    DVP_Image_Validate(&pIO->output, 8, 8, 16, 1, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_SOBEL_3x3_16s:
            case DVP_KN_IMG_SOBEL_5x5_16s:
            case DVP_KN_IMG_SOBEL_7x7_16s:
            case DVP_KN_IMG_PERIMETER_16:
            case DVP_KN_IMG_MEDIAN_3x3_16s:
            case DVP_KN_IMG_FDCT_8x8:
            case DVP_KN_IMG_IDCT_8x8_12Q4:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_YUV422p_TO_RGB565:
            case DVP_KN_IMG_YUV422p_TO_RGB565:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                fourcc_t valid_colors[] = {FOURCC_RGB565};
                if (DVP_Image_Validate(&pIO->input, 2, 2, 1, 1, colorUYVY, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 1, 1, 1, 1, valid_colors, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_PIX_SAT:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_PIX_EXPAND:
            {
                DVP_Transform_t *pIO = dvp_knode_to(&pNodes[n], DVP_Transform_t);
                if (DVP_Image_Validate(&pIO->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pIO->output, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    pIO->input.width > pIO->output.width ||
                    pIO->input.height > pIO->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_IMG_CLIPPING_16:
            {
                DVP_CannyHystThresholding_t *pDth = dvp_knode_to(&pNodes[n], DVP_CannyHystThresholding_t);
                if (DVP_Image_Validate(&pDth->inMag, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pDth->output, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    pDth->inMag.width > pDth->output.width ||
                    pDth->inMag.height > pDth->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_HISTOGRAM_8:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                if (DVP_Image_Validate(&pH->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_HISTOGRAM_16:
            {
                DVP_Histogram_t *pH = dvp_knode_to(&pNodes[n], DVP_Histogram_t);
                if (DVP_Image_Validate(&pH->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_CANNY_IMAGE_SMOOTHING:
            case DVP_KN_CONV_3x3:
            case DVP_KN_CONV_5x5:
            case DVP_KN_CONV_7x7:
            case DVP_KN_IMG_CONV_3x3:
            case DVP_KN_IMG_CONV_5x5:
            case DVP_KN_IMG_CONV_7x7:
            case DVP_KN_IMG_CONV_11x11:
            {
                DVP_ImageConvolution_t *pImg = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_t);
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->mask, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CONV_3x3_I16s_C16:
            case DVP_KN_IMG_CONV_7x7_I16s_C16:
            case DVP_KN_IMG_CONV_11x11_I16s_C16:
            {
                DVP_ImageConvolution_t *pImg = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_t);
                if (DVP_Image_Validate(&pImg->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->mask, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_SAD_3x3:
            case DVP_KN_IMG_SAD_5x5:
            case DVP_KN_IMG_SAD_7x7:
            case DVP_KN_IMG_SAD_8x8:
            case DVP_KN_IMG_SAD_16x16:
            {
                DVP_SAD_t *pImg = dvp_knode_to(&pNodes[n], DVP_SAD_t);
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->refImg, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_MAD_8x8:
            case DVP_KN_IMG_MAD_16x16:
            {
                DVP_SAD_t *pImg = dvp_knode_to(&pNodes[n], DVP_SAD_t);
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 4, 4, 1, 1, colorY32, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->refImg, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CONV_5x5_I8_C16:
            case DVP_KN_IMG_CONV_7x7_I8_C16:
            {
                DVP_ImageConvolution_with_buffer_t *pImg_buff = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_with_buffer_t);
                if (DVP_Image_Validate(&pImg_buff->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg_buff->mask, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg_buff->output, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pImg_buff->scratch) == DVP_FALSE ||
                    pImg_buff->input.width > pImg_buff->output.width ||
                    pImg_buff->input.height > pImg_buff->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CONV_5x5_I16s_C16:
            {
                DVP_ImageConvolution_with_buffer_t *pImg_buff = dvp_knode_to(&pNodes[n], DVP_ImageConvolution_with_buffer_t);
                if (DVP_Image_Validate(&pImg_buff->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg_buff->mask, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg_buff->output, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pImg_buff->scratch) == DVP_FALSE ||
                    pImg_buff->input.width > pImg_buff->output.width ||
                    pImg_buff->input.height > pImg_buff->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_THR_GT2MAX_8:
            case DVP_KN_THR_GT2THR_8:
            case DVP_KN_THR_LE2MIN_8:
            case DVP_KN_THR_LE2THR_8:
            case DVP_KN_IMG_THR_GT2MAX_8:
            case DVP_KN_IMG_THR_GT2THR_8:
            case DVP_KN_IMG_THR_LE2MIN_8:
            case DVP_KN_IMG_THR_LE2THR_8:
            {
                DVP_Threshold_t *pImg = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_THR_GT2MAX_16:
            case DVP_KN_THR_GT2THR_16:
            case DVP_KN_THR_LE2MIN_16:
            case DVP_KN_THR_LE2THR_16:
            case DVP_KN_IMG_THR_GT2MAX_16:
            case DVP_KN_IMG_THR_GT2THR_16:
            case DVP_KN_IMG_THR_LE2MIN_16:
            case DVP_KN_IMG_THR_LE2THR_16:
            case DVP_KN_IMG_ERRDIFF_BIN_16:
            {
                DVP_Threshold_t *pImg = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                if (DVP_Image_Validate(&pImg->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_ERRDIFF_BIN_8:
            {
                DVP_Threshold_t *pImg = dvp_knode_to(&pNodes[n], DVP_Threshold_t);
                if (DVP_Image_Validate(&pImg->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pImg->output, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    pImg->input.width > pImg->output.width ||
                    pImg->input.height > pImg->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_YC_DEMUX_BE16_8:
            case DVP_KN_IMG_YC_DEMUX_LE16_8:
            {
                DVP_Int2Pl_t *pParam = dvp_knode_to(&pNodes[n], DVP_Int2Pl_t);
                if (DVP_Image_Validate(&pParam->input, 2, 2, 1, 1, colorUYVY, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output1, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output2, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pParam->output3, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CORR_3x3:
            {
                DVP_ImageCorrelation_t *pCOR = dvp_knode_to(&pNodes[n], DVP_ImageCorrelation_t);
                if (DVP_Image_Validate(&pCOR->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCOR->mask, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCOR->output, 4, 4, 1, 1, colorY32, 1) == DVP_FALSE ||
                    pCOR->input.width > pCOR->output.width ||
                    pCOR->input.height > pCOR->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CORR_3x3_I8_C16s:
            case DVP_KN_IMG_CORR_11x11_I8_C16s:
            {
                DVP_ImageCorrelation_t *pCOR = dvp_knode_to(&pNodes[n], DVP_ImageCorrelation_t);
                if (DVP_Image_Validate(&pCOR->input, 1, 1, 1, 1, colorY800, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCOR->mask, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCOR->output, 4, 4, 1, 1, colorY32, 1) == DVP_FALSE ||
                    pCOR->input.width > pCOR->output.width ||
                    pCOR->input.height > pCOR->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CORR_3x3_I16s_C16s:
            case DVP_KN_IMG_CORR_5x5_I16s_C16s:
            case DVP_KN_IMG_CORR_11x11_I16s_C16s:
            case DVP_KN_IMG_CORR_GEN_IQ:
            {
                DVP_ImageCorrelation_t *pCOR = dvp_knode_to(&pNodes[n], DVP_ImageCorrelation_t);
                if (DVP_Image_Validate(&pCOR->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCOR->mask, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCOR->output, 4, 4, 1, 1, colorY32, 1) == DVP_FALSE ||
                    pCOR->input.width > pCOR->output.width ||
                    pCOR->input.height > pCOR->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_CORR_GEN_I16s_C16s:
            {
                DVP_ImageCorrelation_t *pCOR = dvp_knode_to(&pNodes[n], DVP_ImageCorrelation_t);
                if (DVP_Image_Validate(&pCOR->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCOR->mask, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pCOR->output, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    pCOR->input.width > pCOR->output.width ||
                    pCOR->input.height > pCOR->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_QUANTIZE_16:
            {
                DVP_ImageQuantization_t *pQnt = dvp_knode_to(&pNodes[n], DVP_ImageQuantization_t);
                if (DVP_Image_Validate(&pQnt->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pQnt->recip_tbl) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_IMG_WAVE_HORZ:
            {
                DVP_ImageWavelet_t *pWv = dvp_knode_to(&pNodes[n], DVP_ImageWavelet_t);
                if (DVP_Image_Validate(&pWv->input, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Image_Validate(&pWv->output, 2, 2, 1, 1, colorY16, 1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pWv->qmf) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pWv->mqmf) == DVP_FALSE ||
                    pWv->input.width > pWv->output.width ||
                    pWv->input.height > pWv->output.height)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
#endif
#ifdef DVP_USE_DSPLIB
            case DVP_KN_DSP_ADD16:
            case DVP_KN_DSP_ADD32:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->input1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_AUTOCORR16:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_BITEXP32:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_BLKESWAP16:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_BLKESWAP32:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_BLKESWAP64:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_BLKMOVE:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_DOTPRODSQR:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->input1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_DOTPROD:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->input1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_FFT_16x16:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_FFT_16x16R:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_FFT_16x16_IMRE:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_FFT_16x32:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_FFT_32x32:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_FFT_32x32s:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_DSP_FIR_CPLX:
            case DVP_KN_DSP_FIR_HM4x4:
            case DVP_KN_DSP_FIR_GEN:
            case DVP_KN_DSP_FIR_HM17_RA8x8:
            case DVP_KN_DSP_FIR_R4:
            case DVP_KN_DSP_FIR_R8:
            case DVP_KN_DSP_FIR_HM16_RM8A8x8:
            case DVP_KN_DSP_FIR_SYM:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_FIR_LMS2:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_DSP_IFFT_16x16:
            case DVP_KN_DSP_IFFT_16x16_IMRE:
            case DVP_KN_DSP_IFFT_16x32:
            case DVP_KN_DSP_IFFT_32x32:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_DSP_IIR:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->input1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_DSP_IIR_LAT:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->input1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_DSP_IIR_SS:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->input1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_DSP_MUL:
            case DVP_KN_DSP_MUL_CPLX:
            {
                DVP_DSP_MatMul *pBuf = dvp_knode_to(&pNodes[n], DVP_DSP_MatMul);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->input1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_MAT_TRANS:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_MAXIDX:
            case DVP_KN_DSP_MAXVAL:
            case DVP_KN_DSP_MINVAL:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_MINERROR:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->input1) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_MUL32:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->input1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_NEG32:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_RECIP16:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->twoway) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_VECSUMSQ:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }
            case DVP_KN_DSP_W_VEC:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->input1) == DVP_FALSE ||
                    DVP_Buffer_Validate(&pBuf->output0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_GEN_TWIDDLE_FFT_16X16:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_GEN_TWIDDLE_FFT_16X16_IMRE:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_GEN_TWIDDLE_FFT_16X32:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_GEN_TWIDDLE_FFT_32x32:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_GEN_TWIDDLE_IFFT_16X16:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE)
                    pNodes[n].header.error = DVP_ERROR_INVALID_PARAMETER;
                break;
            }

            case DVP_KN_GEN_TWIDDLE_IFFT_16X16_IMRE:
            {
                DVP_DSPFunc *pBuf = dvp_knode_to(&pNodes[n], DVP_DSPFunc);
                if (DVP_Buffer_Validate(&pBuf->input0) == DVP_FALSE)
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

static_module_table_t dvp_kgm_dsp_table = {
    "dvp_kgm_dsp",
    dimof(dvp_kgm_functions),
    dvp_kgm_functions
};
#endif
